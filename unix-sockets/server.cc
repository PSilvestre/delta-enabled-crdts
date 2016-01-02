#include "csock.h"

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  int socket_server_fd = helper::net::listen_on(atoi(argv[1]));
  csocketserver socket_server(socket_server_fd);

  while(true)
  {
    map<int, proto::message> fd_to_new_messages;
    socket_server.act(fd_to_new_messages);

    for(const auto& kv : fd_to_new_messages)
      for(auto& client_fd : socket_server.connected())
      {
        csocket socket(client_fd);
        socket.send(kv.second);
      }
  }

  socket_server.end();

  return 0; 
}
