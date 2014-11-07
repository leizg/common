#include "connector.h"

#include "poll.h"

namespace io {

int Connector::CreateSocket() const {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    PLOG(WARNING)<< "socket error";
    return INVALID_FD;
  }

  setFdNonBlock(fd);
  setFdCloExec(fd);

  return fd;
}

bool Connector::WaitForConnected(int fd, uint32 time_out) const {
  pollfd pfd[1];
  ::memset(&pfd[0], 0, sizeof(pollfd));

  pfd[0].fd = fd;
  pfd[0].events = POLLOUT;
  while (true) {
    int ret = ::poll(pfd, 1, 1);
    if (ret == -1) {
      if (errno == EINTR) continue;
      break;
    } else if (ret == 1) {
      uint32 err = 0, len = sizeof(err);
      ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
      if (err == 0) return true;
    }
    break;
  }

  return false;
}

int Connector::Connect(const std::string& ip, uint16 port,
                       uint32 time_out) const {
  int fd = CreateSocket();
  if (fd == INVALID_FD) return INVALID_FD;

  sockaddr_in addr;
  ::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
  addr.sin_addr.s_addr = ::inet_addr(ip.c_str());

  int ret = ::connect(fd, (sockaddr*) &addr, sizeof(addr));
  if (ret == 0) return fd;
  if (errno != EINPROGRESS) {
    ::close(fd);
    PLOG(WARNING)<< "connect error";
    return INVALID_FD;
  }

  TimeStamp time_stamp;
  while (time_out > 0) {
    if (WaitForConnected(fd, time_out)) {
      return fd;
    }

    time_out -= (Now() - time_stamp) / 1000;
  }

  ::close(fd);
  return INVALID_FD;
}

}
