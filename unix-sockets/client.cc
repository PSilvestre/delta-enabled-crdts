#include <string>
#include <thread>
#include "csock.h"

void read_from_socket(csocket& socket)
{
  while(true)
  {
    proto::message message;
    socket.receive(message);
    cout << message.text() << endl;
  }
}

int main(int argc, char *argv[])
{
  if (argc < 3) 
  {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(0);
  }

  csocket socket = connect_to(argv[1], atoi(argv[2]));

  thread socket_reader(read_from_socket, ref(socket));

  string line;
  while(getline(cin, line))
  {
    proto::message message;
    message.set_type(proto::message::CHAT);
    message.set_text(line);
    socket.send(message);
  }

  socket.end();

  return 0;
}
