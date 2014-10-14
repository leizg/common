#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include "base/base.h"

namespace io {

class Connector : public TcpClient::Connector {
 public:
  Connector() {
  }
  virtual ~Connector() {
  }

  int Connect(const std::string& ip, uint16 port, uint32 time_out);

 private:
  int CreateSocket();
  // return true iif connected successfully.
  bool WaitForConnected(int fd, uint32 time_out);

  DISALLOW_COPY_AND_ASSIGN(Connector);
};
}

#endif /* CONNECTOR_H_ */
