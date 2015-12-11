#include <string>
#include <iostream>
#include "crdt.pb.h"
#include "csock.h" 

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  csocketserver socket_server = listen_on(atoi(argv[1]));
  csocket client = socket_server.accept_one();

  proto::crdt crdt = client.receive();
  std::cout << crdt.text() << std::endl;

  client.end();
  socket_server.end();

  return 0; 
}
