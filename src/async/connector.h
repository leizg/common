#pragma once
#include "base/base.h"

namespace async {

class Connector {
  public:
    Connector() {
    }

    // return INVALID_FD if connected failed.
    // timeout: wait for micro seconds.
    int connect(const std::string& path, uint64 timeout) const;
    int connect(const std::string& ip, uint16 port, uint64 timeout) const;

  private:
    bool WaitForConnected(int fd, uint64 time_out) const;

    DISALLOW_COPY_AND_ASSIGN(Connector);
};
}

