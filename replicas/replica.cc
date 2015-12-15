#include <string>
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>
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

void receive_updates(int port, twopset<string>& tps, mutex& mtx)
{
  csocketserver socket_server = listen_on(port);

  while(true)
  {
    vector<proto::message> new_messages;
    socket_server.act(new_messages);

    mtx.lock();
    for(const auto& new_message : new_messages)
    {
      twopset<string> delta;
      new_message >> delta;

      tps.join(delta);
    }
    mtx.unlock();
  }

  socket_server.end();
}

void send_updates(vector<int> other_replicas_ports, twopset<string>& tps, mutex& mtx)
{
  char host[] = "localhost";
  vector<csocket> other_replicas;

  for(const auto& port : other_replicas_ports)
  {
    csocket other_replica = connect_to(host, port);
    other_replicas.push_back(other_replica);
  }

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
        twopset<string> deltas;

        mtx.lock();
        for(int i = 1; i < parts.size(); i++)
        {
          twopset<string> delta;

          if(parts.front() == "add") delta = tps.add(parts.at(i));
          else delta = tps.rmv(parts.at(i));

          deltas.join(delta);
        }
        mtx.unlock();

        proto::message message;
        message << deltas;
        for(auto& other_replica : other_replicas) other_replica.send(message);

      } else if(parts.front() == "show") {
        mtx.lock(); // is this needed?
        cout << tps << endl;
        mtx.unlock();
      } else {
        cout << "Unrecognized option" << endl;
      }
    }
  }

  for (auto& other_replica : other_replicas) other_replica.end();
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s PORT [OTHER_REPLICAS_PORT]\n", argv[0]);
    exit(1);
  }

  twopset<string> gs;
  mutex mtx;
  int port = atoi(argv[1]);
  vector<int> other_replicas_ports;

  for(int i = 2; i < argc; i++) other_replicas_ports.push_back(atoi(argv[i]));

  thread reader(receive_updates, port, std::ref(gs), std::ref(mtx));
  sleep(5);
  thread writer(send_updates, other_replicas_ports, std::ref(gs), std::ref(mtx));

  reader.join();
  writer.join();

  return 0;
}
