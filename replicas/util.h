#include <string>
#include <sstream>
#include <vector>
#include <random>
#include "../unix-sockets/csock.h"

using namespace std;

namespace util {
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
}

