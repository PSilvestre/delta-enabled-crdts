#ifndef HELPERS_H__INCLUDED
#define HELPERS_H__INCLUDED

#include <string>
#include <vector>
#include "message.pb.h"

const int MAX_HEADER_SIZE = 4;
const int BACKLOG_QUEUE_SIZE = 128;

using namespace std;

void error (const char *msg);

namespace helper {
  namespace net {
    int connect_to(char* host, int port);
    int listen_on(int port);
    int accept_one(int socket_fd); // rename to accept
  }

  namespace pb {
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
}

#endif

