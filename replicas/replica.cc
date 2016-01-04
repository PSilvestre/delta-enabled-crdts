// 
// Implementation (not ready) of the anti-entropy algorithm described in
// http://haslab.uminho.pt/ashoker/files/deltacrdt.pdf
// 
#include <unistd.h> // sleep
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <random>
#include "../delta-crdts.cc"
#include "../csock/csocket.h"
#include "../csock/csocketserver.h"
#include "../helpers.h"
#include "../message.pb.h"

using namespace std;

void id_and_port(string s, int& id, int& port);
template <typename T> T random(set<T> s);
template <typename T> T random(vector<T> v);

void socket_reader(int my_id, int& seq, twopset<string>& crdt, map<int, twopset<string>>& seq_to_delta, map<int, int>& id_to_ack, csocketserver& socket_server, mutex& mtx)
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

        if(message.type() == proto::message::TWOPSET)
        {
          // 9 on receive(delta, d, n)
          twopset<string> delta;
          message >> delta;

          if(true)
          {
            // TODO this if should be:
            // 10 if d < Xi

            mtx.lock();
            crdt.join(delta);
            seq_to_delta[seq++] = delta;
            mtx.unlock();
            
            cout << crdt << endl;
          }

          proto::message ack;
          ack.set_type(proto::message::ACK);
          ack.set_id(my_id);
          ack.set_seq(message.seq());

          int replica_fd = socket_server.id_to_fd()[message.id()];
          helper::pb::send(replica_fd, ack);
        }
        else if(message.type() == proto::message::ACK)
        {
          // 15 one receive (ack, n)
          mtx.lock();
          int replica_id = message.id();
          int new_ack = message.seq();
          int current_ack = id_to_ack.count(replica_id) ? id_to_ack[replica_id] : 0;
          int max = current_ack > new_ack ? current_ack : new_ack;

          socket_server.set_id(kv.first, replica_id);
          id_to_ack[replica_id] = max;

          mtx.unlock();
        } else cout << "Can't handle messages with type " << message.type() << endl;
      }
    }
  }
}

void keyboard_reader(int my_id, int& seq, twopset<string>& crdt, map<int, twopset<string>>& seq_to_delta, map<int, int>& id_to_ack, csocketserver& socket_server, mutex& mtx)
{
  cout << "Usage:\n";
  cout << "add [elems]\n";
  cout << "rmv [elems]\n";
  cout << "show\n";
  cout << "connect [unique_id:port]" << endl;

  string line;
  while(getline(cin, line))
  {
    vector<string> parts = helper::str::split(line, ' ');
    if(!parts.empty())
    {
      if(parts.front() == "add" || parts.front() == "rmv")
      {
        // 17 on operation
        twopset<string> delta;

        mtx.lock();
        for(int i = 1; i < parts.size(); i++)
        {
          if(parts.front() == "add") delta.join(crdt.add(parts.at(i)));
          else delta.join(crdt.rmv(parts.at(i)));
        }

        seq_to_delta[seq++] = delta;
        mtx.unlock();

        cout << crdt << endl;
      } 
      else if(parts.front() == "show") cout << crdt << endl;
      else if(parts.front() == "connect")
      {
        char host[] = "localhost";
        for(int i = 1; i < parts.size(); i++)
        {
          int replica_id, replica_port;
          id_and_port(parts.at(i), replica_id, replica_port);
          int replica_fd = helper::net::connect_to(host, replica_port);


          // tell that replica my id
          proto::message message;
          message.set_type(proto::message::ACK);
          message.set_id(my_id);
          message.set_seq(0); // can be zero right?
          helper::pb::send(replica_fd, message);
          
          mtx.lock();
          socket_server.add_fd(replica_fd);
          socket_server.set_id(replica_fd, replica_id);
          mtx.unlock();
        }
      } 
      else cout << "Unrecognized option" << endl;
    }
  }
}

void gossiper(int my_id, int& seq, twopset<string>& crdt, map<int, twopset<string>>& seq_to_delta, map<int, int>& id_to_ack, csocketserver& socket_server, mutex& mtx)
{
  // 22 periodically
  sleep(10);

  int replica_id, replica_fd, this_seq;
  twopset<string> delta;
  bool should_gossip = false;

  map<int, int> id_to_fd = socket_server.id_to_fd();
  if(!id_to_fd.empty())
  {
    mtx.lock();
    set<int> ids = helper::map::keys(id_to_fd);
    replica_id = random(ids);
    replica_fd = id_to_fd[replica_id];

    int last_ack = id_to_ack.count(replica_id) ? id_to_ack[replica_id] : 0;
    int min = *helper::map::keys(seq_to_delta).begin();

    // 24
    bool whole_state = seq_to_delta.empty() || min > last_ack;

    if(whole_state) delta = crdt;
    else
    {
      for(int i = last_ack; i < seq; i++)
        delta.join(seq_to_delta[i]);
    }

    this_seq = seq;
    // 28
    should_gossip = last_ack < seq;
    mtx.unlock();
  }

  if(should_gossip)
  {
    proto::message message;
    message << delta;
    message.set_id(my_id);
    message.set_seq(this_seq);
    helper::pb::send(replica_fd, message);
    cout << "just gossip to " << replica_id << endl;
  }

  gossiper(my_id, seq, crdt, seq_to_delta, id_to_ack, socket_server, mtx);
}

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    cerr << "Usage: " << argv[0] << " unique_id:port" << endl;
    exit(0);
  } 

  int id, port;
  string arg(argv[1]);
  id_and_port(arg, id, port);

  int socket_server_fd = helper::net::listen_on(port);
  csocketserver socket_server(socket_server_fd);

  mutex mtx;

  // 3 durable state:
  int seq = 0;
  twopset<string> crdt;
  // 6 volatile state:
  map<int, twopset<string>> seq_to_delta;
  map<int, int> id_to_ack;

  thread sr(
      socket_reader,
      id,
      ref(seq),
      ref(crdt),
      ref(seq_to_delta),
      ref(id_to_ack),
      ref(socket_server),
      ref(mtx)
  );

  thread kr(
      keyboard_reader,
      id,
      ref(seq),
      ref(crdt),
      ref(seq_to_delta),
      ref(id_to_ack),
      ref(socket_server),
      ref(mtx)
  );

  thread g(
      gossiper,
      id,
      ref(seq),
      ref(crdt),
      ref(seq_to_delta),
      ref(id_to_ack),
      ref(socket_server),
      ref(mtx)
  );

  sr.join();
  kr.join();
  g.join();

  return 0;
}

void id_and_port(string s, int& id, int& port)
{
  vector<string> v = helper::str::split(s, ':');
  id = atoi(v.at(0).c_str());
  port = atoi(v.at(1).c_str());
}

template <typename T>
T random(set<T> s)
{
  vector<T> v;
  for(const auto& e : s) v.push_back(e);
  return random(v);
}

template <typename T>
T random(vector<T> v)
{
  random_device rd;
  default_random_engine e(rd());
  uniform_int_distribution<int> dist(0, v.size() - 1);
  int index = dist(e);
  return v.at(index);
}

