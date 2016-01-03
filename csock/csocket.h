#ifndef CSOCKET_H__INCLUDED
#define CSOCKET_H__INCLUDED

#include "../message.pb.h"

using namespace std;

class csocket
{
  private:
    int socket_fd;

  public:
    csocket(int fd);

    int fd();
    bool send(const proto::message& message);
    bool receive(proto::message& message);
    void end(); // rename to close
};

#endif

