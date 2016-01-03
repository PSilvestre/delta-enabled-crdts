#ifndef CSOCKETSERVER_H__INCLUDED
#define CSOCKETSERVER_H__INCLUDED

#include <set>
#include <map>
#include "../message.pb.h"

using namespace std;

class csocketserver
{
  private:
    int _fd;
    fd_set _active_fd_set;
    fd_set _read_fd_set;
    set<int> _connected_fd;
    map<int, int> _fd_to_id;

  public:
    csocketserver(int fd);

    int fd();
    set<int> connected_fd();
    map<int, int> fd_to_id();
    void add_fd(int fd);
    void remove_fd(int fd);
    void set_id(int fd, int id);

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

