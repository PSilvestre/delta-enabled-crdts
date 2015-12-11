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

void show_crdt_proto(const proto::crdt& crdt)
{
  cout << "byte size: " << crdt.ByteSize() << endl;
  string crdt_str;
  google::protobuf::TextFormat::PrintToString(crdt, &crdt_str);
  cout << crdt_str;
}

void receive_updates(int port, twopset<string>& gs, mutex& mtx)
{
  csocketserver socket_server = listen_on(port);
  csocket other_replica = socket_server.accept_one();

  while(true)
  {
    proto::crdt crdt = other_replica.receive();   
    twopset<string> delta;
    load(crdt, delta);

    mtx.lock();
    gs.join(delta);
    mtx.unlock();
  }

  other_replica.end();
  socket_server.end();
}

void send_updates(int port, twopset<string>& tps, mutex& mtx)
{
  char host[] = "localhost";
  csocket other_replica = connect_to(host, port);

  string line;
  while(getline(cin, line))
  {
    vector<string> parts = split(line, ' ');
    if(parts.size() > 0) {
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

        proto::crdt crdt;
        dump(crdt, deltas);
        other_replica.send(crdt);

      } else if(parts.front() == "show") {
        mtx.lock(); // is this needed?
        cout << tps << endl;
        mtx.unlock();
      } else {
        cout << "Unrecognized option" << endl;
      }
    }
  }

  other_replica.end();
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s PORT OTHER_REPLICA_PORT\n", argv[0]);
    exit(1);
  }

  twopset<string> gs;
  mutex mtx;

  thread reader(receive_updates, atoi(argv[1]), std::ref(gs), std::ref(mtx));
  sleep(5);
  thread writer(send_updates, atoi(argv[2]), std::ref(gs), std::ref(mtx));

  reader.join();
  writer.join();

  return 0;
}
