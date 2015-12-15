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
    vector<proto::message> new_messages;
    socket_server.act(new_messages);

    for(const auto& new_message : new_messages)
      for(auto& client : socket_server.connected()) client.send(new_message);
  }

  socket_server.end();

  return 0; 
}
