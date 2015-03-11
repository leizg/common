#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include "tcp_client.h"

namespace async {

class Connector {
  public:
    Connector() {
    }

    // return INVALID_FD if connected failed.
    // timeout: wait for micro seconds.
    int Connect(const std::string& ip, uint16 port, uint64 timeout) const;

  private:
    int CreateSocket() const;

    // return true iif connected successfully.
    bool WaitForConnected(int fd, uint64 time_out) const;

    DISALLOW_COPY_AND_ASSIGN(Connector);
};
}

#endif /* CONNECTOR_H_ */
