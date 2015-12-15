#include <map>
#include "csock.h" 

void process_message(proto::crdt& crdt, csocketserver& socket_server)
{
  for (auto& client : socket_server.connected()) client.send(crdt);  
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  csocketserver socket_server = listen_on(atoi(argv[1]));

  while(true)
  {
    socket_server.act(process_message);
  }

  socket_server.end();

  return 0; 
}
