#include "crdt.pb.h"
#include "csock.h"

int main(int argc, char *argv[])
{
  if (argc < 3) 
  {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(0);
  }

  csocket socket = connect_to(argv[1], atoi(argv[2]));

  proto::crdt crdt;
  crdt.set_text("ok!");

  socket.send(crdt);
  socket.end();

  return 0;
}
