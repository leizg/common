#include "connector.h"

#include "poll.h"

namespace async {

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

bool Connector::WaitForConnected(int fd, uint64 time_out) const {
  pollfd pfd[1];
  ::memset(&pfd[0], 0, sizeof(pollfd));

  pfd[0].fd = fd;
  pfd[0].events = POLLOUT;
  while (time_out > 0) {
    TimeStamp before(TimeStamp::now());
    int ret = ::poll(pfd, 1, time_out % TimeStamp::kMicroSecsPerMilliSecond);
    if (ret == -1) {
      if (errno == EINTR) {
        TimeStamp delta(TimeStamp::now() -= before);
        time_out -= delta.microSecs();
        continue;
      }
      LOG(WARNING)<< "poll fd error, fd: " << pfd;
    } else if (ret == 1) {
      uint32 err = 0, len = sizeof(err);
      ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
      if (err == 0) return true;
      LOG(WARNING) << "socket error: " << ::strerror(err);
    }
  }

  return false;
}

int Connector::Connect(const std::string& ip, uint16 port,
                       uint64 time_out) const {
  int fd = CreateSocket();
  if (fd == INVALID_FD) return INVALID_FD;

  sockaddr_in addr;
  ::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ::inet_addr(ip.c_str());

  int ret = ::connect(fd, (sockaddr*) &addr, sizeof(addr));
  if (ret == 0) return fd;
  if (errno != EINPROGRESS) {
    ::close(fd);
    PLOG(WARNING)<< "connect error";
    return INVALID_FD;
  }

  if (WaitForConnected(fd, time_out)) {
    return fd;
  }

  ::close(fd);
  return INVALID_FD;
}

}
