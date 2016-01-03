#ifndef CSOCKETSERVER_H__INCLUDED
#define CSOCKETSERVER_H__INCLUDED

#include <vector>
#include <map>
#include "../message.pb.h"

using namespace std;

class csocketserver
{
  private:
    int socket_fd;
    fd_set active_fd_set;
    fd_set read_fd_set;
    vector<int> connected_fd;

  public:
    csocketserver(int fd);

    int fd();
    vector<int> connected();
    void add_fd(int fd);
    void remove_fd(int fd);

    /**
     * This method accepts new clients or receives new messages
     * using select system call
     *
     * if there are new messages, they will be added to map "fd_to_new_messages"
     */
    void act(map<int, proto::message>& fd_to_new_messages);

    void end(); // rename to close
};

#endif

