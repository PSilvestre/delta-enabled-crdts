// 
// Implementation (not ready) of the anti-entropy algorithm described in
// http://haslab.uminho.pt/ashoker/files/deltacrdt.pdf
// 
#include <unistd.h> // sleep
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include "../delta-crdts.cc"
#include "../csock/csocket.h"
#include "../csock/csocketserver.h"
#include "../helpers.h"
#include "../message.pb.h"

using namespace std;

void id_and_port(string& s, int& id, int& port);
void id_host_and_port(string& s, int& id, string& host, int& port);

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

          mtx.lock();
          if(!(crdt == delta))
          {
            // 10 if d > Xi
            // since less than (partial order) is not defined in delta-crdts.cc
            // just test if it's different
            // TODO fix this

            crdt.join(delta);
            seq_to_delta[seq++] = delta;
            
            cout << crdt << endl;
          }
          mtx.unlock();

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
  cout << "connect [unique_id:host:port]\n";
  cout << "show" << endl;

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
          proto::message message;
          message.set_type(proto::message::ACK);
          message.set_id(my_id);
          message.set_seq(0); // can be zero right?
          helper::pb::send(replica_fd, message);
        }
      } 
      else cout << "Unrecognized option" << endl;
    }
  }
}

void garbage_collect_deltas(map<int, twopset<string>>& seq_to_delta, map<int, int>& id_to_ack, mutex& mtx)
{
  if(seq_to_delta.empty()) return; // if nothing to collect

  map<int, twopset<string>> new_seq_to_delta;
  mtx.lock();

  vector<int> acks = helper::map::values(id_to_ack);
  int min = helper::min(acks);

  for(const auto& kv : seq_to_delta)
    if(kv.first >= min)
      new_seq_to_delta.emplace(kv.first, kv.second);

  seq_to_delta = new_seq_to_delta;
  mtx.unlock();
}

void gossiper(int my_id, int& seq, twopset<string>& crdt, map<int, twopset<string>>& seq_to_delta, map<int, int>& id_to_ack, csocketserver& socket_server, mutex& mtx)
{
  sleep(10);

  // 30 garbage collect deltas
  garbage_collect_deltas(seq_to_delta, id_to_ack, mtx);

  // 22 periodically ship delta-interval or state
  int replica_id, replica_fd, this_seq;
  twopset<string> delta;
  bool should_gossip = false;

  map<int, int> id_to_fd = socket_server.id_to_fd();
  if(!id_to_fd.empty())
  {
    mtx.lock();
    set<int> ids = helper::map::keys(id_to_fd);
    replica_id = helper::random(ids);
    replica_fd = id_to_fd[replica_id];

    int last_ack = id_to_ack.count(replica_id) ? id_to_ack[replica_id] : 0;
    set<int> seqs = helper::map::keys(seq_to_delta);
    int min = helper::min(seqs);

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
