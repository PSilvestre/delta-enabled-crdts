#include <unistd.h>
#include <mutex>
#include <thread>
#include <random>
#include <google/protobuf/text_format.h>
#include "../csock/csocket.h"
#include "../csock/csocketserver.h"
#include "../helpers.h"
#include "../delta-crdts.cc"

using namespace std;

void id_and_port(char arg[], int& id, int& port)
{
  string s(arg);
  vector<string> v = helper::str::split(s, ':');
  id = atoi(v.at(0).c_str());
  port = atoi(v.at(1).c_str());
}

int random_replica_id(map<int, csocket>& replicas)
{
  random_device rd;
  default_random_engine e(rd());
  uniform_int_distribution<int> dist(0, replicas.size() - 1);
  int index = dist(e);

  vector<int> replicas_id;
  for(auto& kv : replicas) replicas_id.push_back(kv.first);

  return replicas_id.at(index);
}

void socket_reader(int my_id, int port, twopset<string>& tps, int& seq, map<int, twopset<string>>& deltas, map<int, int>& acks, mutex& mtx)
{
  int socket_server_fd = helper::net::listen_on(port);
  csocketserver socket_server(socket_server_fd);

  while(true)
  {
    map<int, proto::message> new_messages;
    socket_server.act(new_messages);

    if(!new_messages.empty())
    {
      map<int, proto::message> acks_later;

      mtx.lock();
      for(const auto& kv : new_messages)
      {
        proto::message new_message = kv.second;

        if(new_message.type() == proto::message::ACK)
        {
          int id = new_message.id();
          int max;
          if(acks.count(id) > 0 && acks.at(id) > new_message.seq()) max = acks.at(id);
          else max = new_message.seq();

          acks[id] = max;
        }
        else
        {
          twopset<string> delta;
          new_message >> delta;

          tps.join(delta);
          deltas[seq++] = delta;

          proto::message ack;
          ack.set_type(proto::message::ACK);
          ack.set_id(my_id);
          ack.set_seq(new_message.seq());
          acks_later[kv.first] = ack;
        }
      }
      mtx.unlock();
      
      for(auto& kv : acks_later)
        helper::pb::send(kv.first, kv.second);
    }
  }

  socket_server.end();
}

void keyboard_reader(twopset<string>& tps, int& seq, map<int, twopset<string>>& deltas, mutex& mtx)
{
  cout << "Usage:\n";
  cout << "add [elems]\n";
  cout << "rmv [elems]\n";
  cout << "show" << endl;

  string line;
  while(getline(cin, line))
  {
    vector<string> parts = helper::str::split(line, ' ');
    if(!parts.empty()) {
      if(parts.front() == "add" || parts.front() == "rmv") {
        twopset<string> delta;

        mtx.lock();
        for(int i = 1; i < parts.size(); i++)
        {
          string elem = parts.at(i);

          if(parts.front() == "add") 
            delta.join(tps.add(elem));
          else 
            delta.join(tps.rmv(elem));
        }

        deltas[seq++] = delta;
        mtx.unlock();

      } else if(parts.front() == "show") {
        // is mtx.lock() needed?
        cout << tps << endl;
      } else {
        cout << "Unrecognized option" << endl;
      }
    }
  }
}

void gossip(int my_id, map<int, csocket>& replicas, int& seq, map<int, twopset<string>>& deltas, map<int, int>& acks, mutex& mtx)
{
  sleep(10);
  
  int replica_id = random_replica_id(replicas);
  cout << "will gossip to " << replica_id << endl; 
  
  mtx.lock();
  twopset<string> delta_group;
  int last_ack = acks.count(replica_id) > 0 
    ? acks.at(replica_id)
    : 0;
  
  for (int l = last_ack; l < seq; l++)
    delta_group.join(deltas.at(l));
  
  bool should_gossip = last_ack < seq;
  mtx.unlock();
 
  if(should_gossip)
  { 
    proto::message message;
    message << delta_group;
    message.set_id(my_id);
    message.set_seq(seq);
    replicas.at(replica_id).send(message);
  }

  gossip(my_id, replicas, seq, deltas, acks, mtx);
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s UNIQUE_ID:PORT [UNIQUE_ID:OTHER_REPLICAS_PORT]\n", argv[0]);
    exit(1);
  }

  twopset<string> tps;
  int seq = 0;
  map<int, twopset<string>> deltas; // seq -> delta
  map<int, int> acks; // unique-id -> ack
  mutex mtx;

  char host[] = "localhost";
  int id;
  int port;
  id_and_port(argv[1], id, port);
  map<int, csocket> replicas; // unique-id -> socket

  thread sr(
      socket_reader, 
      id,
      port, 
      std::ref(tps), 
      std::ref(seq), 
      std::ref(deltas), 
      std::ref(acks),
      std::ref(mtx)
  );
  sleep(5);

  for(int i = 2; i < argc; i++) 
  {
    int replica_id;
    int replica_port;
    id_and_port(argv[i], replica_id, replica_port);
    int replica_fd = helper::net::connect_to(host, replica_port);
    csocket replica(replica_fd);
    replicas.emplace(replica_id, replica);
  }

  thread kr(
      keyboard_reader, 
      std::ref(tps), 
      std::ref(seq), 
      std::ref(deltas), 
      std::ref(mtx)
  );

  thread g(
      gossip,
      id,
      std::ref(replicas),
      std::ref(seq),
      std::ref(deltas),
      std::ref(acks),
      std::ref(mtx)
  );

  sr.join();
  kr.join();
  g.join();

  for (auto& kv : replicas) kv.second.end();
  return 0;
}

