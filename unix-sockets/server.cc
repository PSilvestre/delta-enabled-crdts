#include "csock.h" 

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
    map<int, proto::message> new_messages;
    socket_server.act(new_messages);

    for(const auto& kv : new_messages)
      for(auto& client : socket_server.connected()) client.send(kv.second);
  }

  socket_server.end();

  return 0; 
}
