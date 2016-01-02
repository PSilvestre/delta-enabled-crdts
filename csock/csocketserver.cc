#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm> // remove
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

int csocketserver::accept_one() // rename to accept
{
  return helper::net::accept_one(socket_fd);
}

vector<int> csocketserver::connected() 
{ 
  return connected_fd; 
} 

void csocketserver::end() // rename to close
{
  for(auto& fd : connected_fd) close(fd);
  close(socket_fd);
}

void csocketserver::act(map<int, proto::message>& fd_to_new_messages)
{
  read_fd_set = active_fd_set;

  int select_result = select(
      FD_SETSIZE,
      &read_fd_set,
      NULL, NULL, NULL
      );  

  if (select_result < 0) error("error on select");

  for (int i = 0; i < FD_SETSIZE; i++)
  {
    if (FD_ISSET(i, &read_fd_set))
    {
      if( i == socket_fd) // new connection
      {
        int client_socket_fd = accept_one();
        FD_SET(client_socket_fd, &active_fd_set);

        connected_fd.push_back(client_socket_fd);
      }
      else
      {
        proto::message message;
        bool success = helper::pb::receive(i, message);

        if (success) fd_to_new_messages.emplace(i, message);
        else
        {
          close(i);
          FD_CLR(i, &active_fd_set);
          connected_fd.erase(remove(connected_fd.begin(), connected_fd.end(), i), connected_fd.end());
        }
      } 
    }
  }
}

