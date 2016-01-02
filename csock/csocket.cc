#include <unistd.h>
#include "csocket.h"
#include "../helpers.h"

using namespace std;

csocket::csocket(int fd) : socket_fd(fd) {}

int csocket::fd()
{
  return socket_fd; 
}

bool csocket::send(const proto::message& message)
{
  return helper::pb::send(socket_fd, message);
}

bool csocket::receive(proto::message& message)
{
  return helper::pb::receive(socket_fd, message);
}

void csocket::end()
{
  close(socket_fd);
}

