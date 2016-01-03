#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include "csocketserver.h"
#include "../helpers.h"

using namespace std;

csocketserver::csocketserver(int fd) : socket_fd(fd) 
{
  FD_ZERO(&active_fd_set);
  FD_SET(fd, &active_fd_set);
}

int csocketserver::fd() 
{ 
  return socket_fd; 
}

set<int> csocketserver::connected() 
{ 
  return connected_fd; 
} 

set<int> csocketserver::fds_with_id()
{
  return helper::map::keys(fd_to_id);
}

void csocketserver::add_fd(int client_socket_fd)
{
  FD_SET(client_socket_fd, &active_fd_set);
  connected_fd.insert(client_socket_fd);
}

void csocketserver::remove_fd(int client_socket_fd)
{
  FD_CLR(client_socket_fd, &active_fd_set);
  connected_fd.erase(client_socket_fd);
}

void csocketserver::set_id(int fd, int id)
{
  fd_to_id[fd] = id;
}

void csocketserver::act(map<int, proto::message>& fd_to_new_messages)
{
  read_fd_set = active_fd_set;

  struct timeval timeout; // 10 seconds
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;


  int select_result = select(
      FD_SETSIZE,
      &read_fd_set,
      NULL, NULL,
      &timeout
  );  

  if (select_result < 0) error("error on select");
  
  if (select_result == 0) return; // timeout

  for (int i = 0; i < FD_SETSIZE; i++)
  // if two messages from the same fd
  // will I loose one of them?
  {
    if (FD_ISSET(i, &read_fd_set))
    {
      if( i == socket_fd) // new connection
      {
        int client_socket_fd = helper::net::accept_one(socket_fd);
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
  for(auto& fd : connected_fd) close(fd);
  close(socket_fd);
}

