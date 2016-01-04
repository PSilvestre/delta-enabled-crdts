#ifndef HELPERS_H__INCLUDED
#define HELPERS_H__INCLUDED

#include <string>
#include <vector>
#include <set>
#include <map>
#include <random>
#include "message.pb.h"

const int MAX_HEADER_SIZE = 4;
const int BACKLOG_QUEUE_SIZE = 128;

using namespace std;

void error(const char *msg);

namespace helper {
  template<typename T>
  T random(const vector<T>& v)
  {
    random_device rd;
    default_random_engine e(rd());
    uniform_int_distribution<int> dist(0, v.size() - 1);
    int index = dist(e);
    return v.at(index);
  }

  template<typename T>
  T random(const set<T>& s)
  {
    vector<T> v;
    for(const auto& e : s) v.push_back(e);
    return random(v);
  }

  template<typename T>
  T min(const set<T>& s)
  {
    return *s.begin();
  }

  template<typename T>
  T min(const vector<T>& v)
  {
    set<T> s;
    for(const auto& e : v) s.insert(e);
    return min(s);
  }

  namespace net {
    int connect_to(char* host, int port);
    int listen_on(int port);
    int accept_one(int socket_fd); // rename to accept
  }

  namespace pb {
    void show(const proto::message& message);
    int header_size(const uint32_t& body_size);
    uint32_t decode_header(char header_buffer[MAX_HEADER_SIZE]);
    void join(char* message, char* header_buffer, char* body_buffer, int missing_bytes);
    bool send(int socket_fd, const proto::message& message);
    bool receive(int socket_fd, proto::message& message);
  }

  namespace str {
    // string split
    // http://stackoverflow.com/a/236803/4262469
    vector<string>& split(const string& s, char delim, vector<string>& elems);
    vector<string> split(const string& s, char delim);
  }

  namespace map {
    template<typename K, typename V>
    set<K> keys(const std::map<K,V>& map)
    {
      set<K> keys;
      for(const auto& kv : map) keys.insert(kv.first);
      return keys;
    }

    template<typename K, typename V>
    vector<V> values(const std::map<K,V>& map)
    {
      vector<V> values;
      for(const auto& kv : map) values.push_back(kv.second);
      return values;
    }
  }
}

#endif

