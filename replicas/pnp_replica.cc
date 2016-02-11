// 
// Implementation (not ready) of the anti-entropy algorithm described in
// http://haslab.uminho.pt/ashoker/files/deltacrdt.pdf
// 
#include <unistd.h> // sleep
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <time.h>
#include <utility> // pair
#include "../delta-crdts.cc"
#include "../csock/csocket.h"
#include "../csock/csocketserver.h"
#include "../helpers.h"
#include "../message.pb.h"

using namespace std;
bool REPL = false;
bool DELTA = true;

void id_and_port(string& s, int& id, int& port);
void id_host_and_port(string& s, int& id, string& host, int& port);

void show_usage();
void show_crdt(twopset<string>& crdt);

time_t now();
void log_message_sent();
void log_bytes_received(int from, proto::message& message);
void log_pull(int to, proto::message& message);
void log_new_state(twopset<string>& crdt);
void log_op(string& op);


void socket_reader(int my_id, int& seq, twopset<string>& crdt, map<pair<int, int>, twopset<string>>& from_seq_to_delta, map<int, int>& id_to_seq, csocketserver& socket_server, mutex& mtx)
{
  while(true)
  {
    map<int, proto::message> fd_to_new_messages;
    socket_server.act(fd_to_new_messages);

    if(!fd_to_new_messages.empty())
    {
      for(const auto& kv : fd_to_new_messages)
      {
        proto::message message = kv.second;
        int from_id = socket_server.get_id(kv.first);
        int replica_id = message.id();
        int message_seq = message.seq();

        log_bytes_received(from_id, message);

        if(message.type() == proto::message::TWOPSET)
        {
          twopset<string> delta;
          message >> delta;

          mtx.lock();
          if(!(delta <= crdt))
          {
            crdt.join(delta);
            from_seq_to_delta[make_pair(replica_id, message_seq)] = delta;
            id_to_seq[replica_id] = message_seq;
            
            proto::message news_message;
            news_message.set_type(proto::message::NEWS);
            news_message.set_id(replica_id);
            news_message.set_seq(message_seq);

            set<int> ids = helper::map::keys(socket_server.id_to_fd());
            ids.erase(replica_id);

	    for(auto& neighbour_id : ids)
	    {
	      int neighbour_fd = socket_server.get_fd(neighbour_id);
	      helper::pb::send(neighbour_fd, news_message);
	    }

            log_new_state(crdt);
            show_crdt(crdt);
          }
          mtx.unlock();
        }
        else if(message.type() == proto::message::ID)
        {
          mtx.lock();
          socket_server.set_id(kv.first, replica_id);
          mtx.unlock();
        } else if(message.type() == proto::message::NEWS)
        {
          int last_seq = id_to_seq.count(replica_id) > 0 ? id_to_seq[replica_id] : -1;
          if(last_seq < message_seq)
          {
            proto::message pull_message;
            pull_message.set_type(proto::message::PULL);
            pull_message.set_id(replica_id);
            pull_message.set_seq(message_seq);
            helper::pb::send(kv.first, pull_message);
            log_pull(from_id, pull_message);
          }
        } else if(message.type() == proto::message::PULL)
        {
          proto::message push_message;
          push_message << from_seq_to_delta[make_pair(replica_id, message_seq)];
          push_message.set_id(replica_id);
          push_message.set_seq(message_seq);
          helper::pb::send(kv.first, push_message);
          log_message_sent();
        } else cout << "Can't handle messages with type " << message.type() << endl;
      }
    }
  }
}

void keyboard_reader(int my_id, int& seq, twopset<string>& crdt, map<pair<int, int>, twopset<string>>& from_seq_to_delta, map<int, int>& id_to_seq, csocketserver& socket_server, mutex& mtx)
{
  show_usage();

  string line;
  while(getline(cin, line))
  {
    vector<string> parts = helper::str::split(line, ' ');
    if(!parts.empty())
    {
      log_op(line);
      if(parts.front() == "add" || parts.front() == "rmv")
      {
        twopset<string> delta;

        mtx.lock();
        for(int i = 1; i < parts.size(); i++)
        {
          if(parts.front() == "add") delta.join(crdt.add(parts.at(i)));
          else delta.join(crdt.rmv(parts.at(i)));
        }

        show_crdt(crdt);
	
	from_seq_to_delta[make_pair(my_id, seq)] = delta;

        proto::message message;
        message << delta;
        message.set_id(my_id);
        message.set_seq(seq++);

        set<int> ids = helper::map::keys(socket_server.id_to_fd());
    	for(auto& replica_id : ids)
    	{
      	  int replica_fd = socket_server.get_fd(replica_id);
          helper::pb::send(replica_fd, message);
    	  log_message_sent();
        }

        mtx.unlock();
      } 
      else if(parts.front() == "show") cout << crdt << endl;
      else if(parts.front() == "connect")
      {
        for(int i = 1; i < parts.size(); i++)
        {
          int replica_id, replica_port;
          string host;
          id_host_and_port(parts.at(i), replica_id, host, replica_port);

          char* host_ = new char[host.length() + 1];
          strcpy(host_, host.c_str());

          int replica_fd = helper::net::connect_to(host_, replica_port);

          mtx.lock();
          socket_server.add_fd(replica_fd);
          socket_server.set_id(replica_fd, replica_id);
          mtx.unlock();

          // tell that replica my id
          proto::message id_message;
          id_message.set_type(proto::message::ID);
          id_message.set_id(my_id);
          id_message.set_seq(0); // can be zero right?
          helper::pb::send(replica_fd, id_message);
        }
      } 
      else if(parts.front() == "wait" && parts.size() > 1)
      {
        int seconds = atoi(parts.at(1).c_str());
        sleep(seconds);
      }
      else cout << "Unrecognized option" << endl;
    }
  }
}

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    cerr << "Usage: " << argv[0] << " unique_id:port [-r] [-d] [-s]" << endl;
    exit(0);
  } 

  int id, port;
  string arg(argv[1]);
  id_and_port(arg, id, port);

  for(int i = 2; i < argc; i++)
  {
    string arg(argv[i]);
    if(arg == "-r") REPL = true;
    else if(arg == "-d") DELTA = true;
    else if(arg == "-s") DELTA = false;
    // TODO deal with bad usage
  }

  int socket_server_fd = helper::net::listen_on(port);
  csocketserver socket_server(socket_server_fd);

  mutex mtx;

  // 3 durable state:
  int seq = 0;
  twopset<string> crdt;
  // 6 volatile state:
  map<pair<int, int>, twopset<string>> from_seq_to_delta;
  map<int, int> id_to_seq;

  thread sr(
      socket_reader,
      id,
      ref(seq),
      ref(crdt),
      ref(from_seq_to_delta),
      ref(id_to_seq),
      ref(socket_server),
      ref(mtx)
  );

  keyboard_reader(
      id,
      ref(seq),
      ref(crdt),
      ref(from_seq_to_delta),
      ref(id_to_seq),
      ref(socket_server),
      ref(mtx)
  );

  sr.join();

  return 0;
}

void id_and_port(string& s, int& id, int& port)
{
  vector<string> v = helper::str::split(s, ':');
  id = atoi(v.at(0).c_str());
  port = atoi(v.at(1).c_str());
}

void id_host_and_port(string& s, int& id, string& host, int& port)
{
  vector<string> v = helper::str::split(s, ':');

  id = atoi(v.at(0).c_str());
  host = v.at(1);
  port = atoi(v.at(2).c_str());
}

mutex stdout_mtx;
void l() { stdout_mtx.lock(); }
void ul() { stdout_mtx.unlock(); }

void show_usage()
{
  if(!REPL) return;
  l();
  cout << "Usage:\n";
  cout << "add [elems]\n";
  cout << "rmv [elems]\n";
  cout << "show\n";
  cout << "connect [unique_id:host:port]\n";
  cout << "wait seconds" << endl;
  ul();
}

void show_crdt(twopset<string>& crdt)
{
  if(!REPL) return;
  l();
  cout << crdt << endl;
  ul();
}

time_t now()
{
  time_t timer;
  time(&timer);
  return timer;
}

void log_message_sent()
{
  if(REPL) return;
  l();
  cout << now() << "|L|" << endl;
  ul();
}

void log_bytes_received(int from, proto::message& message)
{
  if(REPL) return;
  l();
  cout << now();
  if(message.type() == proto::message::TWOPSET)
  {
    cout << "|B|D|";
    twopset<string> crdt;
    message >> crdt;
    for(auto& e : crdt.read())
      cout << e << ",";
  }
  else if(message.type() == proto::message::ID) cout << "|B|I|" << from;
  else if(message.type() == proto::message::NEWS) cout << "|B|N|" << from;
  else if(message.type() == proto::message::PULL) cout << "|B|P|" << from;
  else
  {
    cout << "Hmm, there's something wrong xD\n";
    helper::pb::show(message);
    cout << endl;
  }
  cout << "|" << message.id() << "|" << message.seq() << "|" << message.ByteSize() << endl;
  ul();
}

void log_pull(int from, proto::message& message)
{
  if(REPL) return;
  l();
  cout << now();
  cout << "|P|" << from << "|" << message.id() << "|" << message.seq() << endl;
  ul();
}

void log_new_state(twopset<string>& crdt)
{
  if(REPL) return;
  l();
  cout << now() << "|S|";
  for(const auto& e : crdt.read())
    cout << e << ",";
  cout << endl;
  ul();
}

void log_op(string& op)
{
  if(REPL) return;
  l();
  cout << now() << "|O|" << op << endl; 
  ul();
}
