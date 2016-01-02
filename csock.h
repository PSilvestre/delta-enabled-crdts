#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm> // remove
#include "helpers.h"

using namespace std;

class csocket
{
  private:
    int socket_fd;

  public:
    csocket(int fd) : socket_fd(fd) {}

    int fd() 
    { 
      return socket_fd; 
    }

    bool send(const proto::message& message)
    {
      return helper::pb::send(socket_fd, message);
    }

    bool receive(proto::message& message)
    {
      return helper::pb::receive(socket_fd, message);
    }

    void end() // rename to close
    {
      close(socket_fd);
    }
};

class csocketserver
{
  private:
    int socket_fd;
    fd_set active_fd_set;
    fd_set read_fd_set;
    vector<int> connected_fd;

  public:
    csocketserver(int fd) : socket_fd(fd) {
      FD_ZERO(&active_fd_set);
      FD_SET(fd, &active_fd_set);
    }

    int fd() 
    { 
      return socket_fd; 
    }

    int accept_one() // rename to accept
    {
      return helper::net::accept_one(socket_fd);
    }

    /**
     * This method accepts new clients or receives new messages
     * using select system call
     *
     * if there are new messages, they will be added to map "fd_to_new_messages"
     */
    void act(map<int, proto::message>& fd_to_new_messages)
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

    vector<int> connected() 
    { 
      return connected_fd; 
    } 

    void end() // rename to close
    {
      for(auto& fd : connected_fd) close(fd);
      close(socket_fd);
    }
};
