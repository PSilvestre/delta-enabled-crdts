#include <map>
#include "csock.h" 

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  map<int, csocket> clients;

  fd_set active_fd_set, read_fd_set;

  csocketserver socket_server = listen_on(atoi(argv[1]));

  FD_ZERO(&active_fd_set);
  FD_SET(socket_server.fd(), &active_fd_set);

  while(true)
  {
    read_fd_set = active_fd_set;

    int select_result = select(
        FD_SETSIZE,
        &read_fd_set,
        NULL, NULL, NULL
    );

    if(select_result < 0) error("error on select");

    for(int i = 0; i < FD_SETSIZE; i++)
    {
      if(FD_ISSET(i, &read_fd_set))
      {
        if(i == socket_server.fd())
        {
          csocket client = socket_server.accept_one();
          FD_SET(client.fd(), &active_fd_set);
          clients.emplace(client.fd(), client);
        } 
        else 
        {
          proto::crdt crdt;
          bool success = clients.at(i).receive(crdt);

          if(!success)
          {
            csocket dead_client = clients.at(i);
            dead_client.end();
            FD_CLR(dead_client.fd(), &active_fd_set);
            clients.erase(dead_client.fd());
          } 
          else for(auto& kv : clients) kv.second.send(crdt);
        }
      }
    }
  }

  for(auto& kv : clients) kv.second.end();
  socket_server.end();

  return 0; 
}
