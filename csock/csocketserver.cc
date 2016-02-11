#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include "csocketserver.h"
#include "../helpers.h"

using namespace std;

csocketserver::csocketserver(int fd) : _fd(fd) 
{
  FD_ZERO(&_active_fd_set);
  FD_SET(fd, &_active_fd_set);
}

int csocketserver::fd() 
{ 
  return _fd; 
}

set<int> csocketserver::connected_fd() 
{ 
  return _connected_fd; 
} 

map<int, int> csocketserver::id_to_fd()
{
  return _id_to_fd;
}

int csocketserver::get_id(int fd)
{
  for(const auto& kv : _id_to_fd)
    if(kv.second == fd) return kv.first;

  return -1;
}

int csocketserver::get_fd(int id)
{
  return _id_to_fd[id];
}

void csocketserver::add_fd(int client_socket_fd)
{
  FD_SET(client_socket_fd, &_active_fd_set);
  _connected_fd.insert(client_socket_fd);
}

void csocketserver::remove_fd(int client_socket_fd)
{
  FD_CLR(client_socket_fd, &_active_fd_set);
  _connected_fd.erase(client_socket_fd);
}

void csocketserver::set_id(int fd, int id)
{
  _id_to_fd[id] = fd;
}

void csocketserver::act(map<int, proto::message>& fd_to_new_messages)
{
  _read_fd_set = _active_fd_set;

  struct timeval timeout; // 10 seconds
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  int select_result = select(
      FD_SETSIZE,
      &_read_fd_set,
      NULL, NULL,
      &timeout
  );  

  if (select_result < 0) error("error on select");
  
  if (select_result == 0) return; // timeout

  for (int i = 0; i < FD_SETSIZE; i++)
  // if two messages from the same fd
  // will I loose one of them?
  {
    if (FD_ISSET(i, &_read_fd_set))
    {
      if(i == _fd) // new connection
      {
        int client_socket_fd = helper::net::accept_one(_fd);
        add_fd(client_socket_fd);
      }
      else
      {
        proto::message message;
        bool success = helper::pb::receive(i, message);

        if (success) fd_to_new_messages.emplace(i, message);
        else
        {
          remove_fd(i);
          close(i);
        }
      } 
    }
  }
}

void csocketserver::end() // rename to close
{
  for(auto& fd : _connected_fd) close(fd);
  close(_fd);
}

