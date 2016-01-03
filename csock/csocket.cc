#include <unistd.h>
#include "csocket.h"
#include "../helpers.h"

using namespace std;

csocket::csocket(int fd) : _fd(fd) {}

int csocket::fd()
{
  return _fd; 
}

bool csocket::send(const proto::message& message)
{
  return helper::pb::send(_fd, message);
}

bool csocket::receive(proto::message& message)
{
  return helper::pb::receive(_fd, message);
}

void csocket::end()
{
  close(_fd);
}

