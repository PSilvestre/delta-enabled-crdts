#include <string>
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>
#include <random>
#include <google/protobuf/text_format.h>
#include "../unix-sockets/csock.h"
#include "../delta-crdts.cc"

using namespace std;

// string split
// http://stackoverflow.com/a/236803/4262469
vector<string>& split(const string& s, char delim, vector<string>& elems)
{
  stringstream ss(s);
  string item;

  while(getline(ss, item, delim)) elems.push_back(item);
  return elems;
}

vector<string> split(const string& s, char delim)
{
  vector<string> elems;
  split(s, delim, elems);
  return elems;
}

void socket_reader(int my_id, int port, twopset<string>& tps, int& seq, map<int, twopset<string>>& deltas, map<int, int>& acks, mutex& mtx)
{
  csocketserver socket_server = listen_on(port);

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
        socket_server.send_to(kv.first, kv.second);
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
    vector<string> parts = split(line, ' ');
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

void gossip(int my_id, vector<csocket>& other_replicas, int& seq, map<int, twopset<string>>& deltas, map<int, int>& acks, mutex& mtx)
{
  sleep(10);
  random_device rd;
  default_random_engine e(rd());
  uniform_int_distribution<int> dist(0, other_replicas.size() - 1);
  int index = dist(e);
  cout << "will gossip to " << index << endl; 
  
  mtx.lock();
  twopset<string> delta_group;
  // this is wrong
  for(auto& kv : deltas) delta_group.join(kv.second);
  mtx.unlock();

  proto::message message;
  message << delta_group;
  message.set_id(my_id);
  message.set_seq(seq);
  other_replicas.at(index).send(message);

  gossip(my_id, other_replicas, seq, deltas, acks, mtx);
}

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: %s UNIQUE_ID PORT [OTHER_REPLICAS_PORT]\n", argv[0]);
    exit(1);
  }

  twopset<string> tps;
  int seq = 0;
  map<int, twopset<string>> deltas; // seq -> delta
  map<int, int> acks; // unique-id -> ack
  mutex mtx;

  char host[] = "localhost";
  int id = atoi(argv[1]);
  int port = atoi(argv[2]);
  vector<csocket> other_replicas;

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

  for(int i = 3; i < argc; i++) 
  {
    int replica_port = atoi(argv[i]);
    csocket other_replica = connect_to(host, replica_port);
    other_replicas.push_back(other_replica);
  }

  thread kr(
      keyboard_reader, 
      std::ref(tps), 
      std::ref(seq), 
      std::ref(deltas), 
      std::ref(mtx)
  );

  thread gossiper(
      gossip,
      id,
      std::ref(other_replicas),
      std::ref(seq),
      std::ref(deltas),
      std::ref(acks),
      std::ref(mtx)
  );

  sr.join();
  kr.join();
  gossiper.join();

  for (auto& other_replica : other_replicas) other_replica.end();
  return 0;
}
