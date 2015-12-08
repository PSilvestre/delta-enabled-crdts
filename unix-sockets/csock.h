#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // constains structure "struct hostent" 
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "crdt.pb.h"

using namespace std;
using google::protobuf::io::ArrayOutputStream;
using google::protobuf::io::ArrayInputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::io::CodedInputStream;

const int BACKLOG_QUEUE_SIZE = 128;
const int MAX_HEADER_SIZE = 4;

void error (const char *msg)
{
  cerr << msg << endl;
  exit(0);
}

class csocket
{
  private:
    int socket_fd;

  public:
    csocket(int fd) : socket_fd(fd) {}

    bool send(proto::crdt& crdt)
    {
      int body_size = crdt.ByteSize();
      int header_size = CodedOutputStream::VarintSize32(body_size);
      int message_size = header_size + body_size;
      char write_buffer[message_size];

      ArrayOutputStream aos(write_buffer, message_size);
      CodedOutputStream cos(&aos);

      cos.WriteVarint32(body_size);
      crdt.SerializeToCodedStream(&cos);

      int bytes_sent = write(socket_fd, write_buffer, message_size);
      return message_size == bytes_sent;
    }

    proto::crdt receive()
    {
      proto::crdt crdt;

      char header_buffer[MAX_HEADER_SIZE];
      int bytes_received = read(socket_fd, header_buffer, MAX_HEADER_SIZE);
      if (bytes_received != MAX_HEADER_SIZE) error("error reading header");

      uint32_t body_size = decode_header(header_buffer);
      int header_size = CodedOutputStream::VarintSize32(body_size);
      int message_size = header_size + body_size;
      int missing_bytes = message_size - MAX_HEADER_SIZE;

      char body_buffer[missing_bytes];
      bytes_received = read(socket_fd, body_buffer, missing_bytes);
      if (bytes_received != missing_bytes) error("error reading body");

      char message[message_size];
      join(message, header_buffer, body_buffer, missing_bytes);

      ArrayInputStream ais(message, message_size);
      CodedInputStream cis(&ais);
      cis.ReadVarint32(&body_size);

      CodedInputStream::Limit limit = cis.PushLimit(body_size);
      crdt.ParseFromCodedStream(&cis);
      cis.PopLimit(limit);

      return crdt;
    }

    void end() // rename to close
    {
      close(socket_fd);
    }

  private:
    uint32_t decode_header(char header_buffer[MAX_HEADER_SIZE])
    {
      uint32_t body_size;

      ArrayInputStream ais(header_buffer, MAX_HEADER_SIZE);
      CodedInputStream cis(&ais);
      cis.ReadVarint32(&body_size);

      return body_size;
    }

    void join(char* message, char* header_buffer, char* body_buffer, int missing_bytes)
    {
      int j = 0;

      for (int i = 0; i < MAX_HEADER_SIZE; i++)
      {
        message[j++] = header_buffer[i];
      }

      for (int i = 0; i < missing_bytes; i++)
      {
       message[j++] = body_buffer[i];
      }
    }
};

class csocketserver
{
  private:
    int socket_fd;

  public:
    csocketserver(int fd) : socket_fd(fd) {}

    csocket accept_one() // rename to accept
    {
      struct sockaddr_in client_address;
      socklen_t client_address_size = sizeof(client_address);

      int client_socket_fd = accept(
          socket_fd,
          (struct sockaddr *) &client_address,
          &client_address_size
      );
      if (client_socket_fd < 0) error("error accepting client");

      csocket socket(client_socket_fd);
      return socket;
    }

    void end() // rename to close
    {
      close(socket_fd);
    }
};

csocket connect_to(char* host, int port)
{
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) error("error opening socket"); 

  struct hostent *server;
  server = gethostbyname(host);
  if (server == NULL) error("no such host");

  struct sockaddr_in server_address;
  bzero((char *) &server_address, sizeof(server_address));

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  bcopy(
      (char *) server->h_addr,
      (char *) &server_address.sin_addr.s_addr,
      server->h_length
  );

  int connect_result = connect(
      socket_fd,
      (struct sockaddr *) &server_address,
      sizeof(server_address)
  );
  if (connect_result < 0) error("error connecting to server");

  csocket socket(socket_fd);
  return socket;
}

csocketserver listen_on(int port)
{
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) error("error opening socket");

  struct sockaddr_in server_address;
  bzero((char *) &server_address, sizeof(server_address));

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = INADDR_ANY;

  int bind_result = bind(
      socket_fd,
      (struct sockaddr *) &server_address,
      sizeof(server_address)
  );
  if (bind_result < 0) error("error binding on port");

  listen(socket_fd, BACKLOG_QUEUE_SIZE);

  csocketserver socket_server(socket_fd);
  return socket_server;
}
