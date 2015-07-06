#pragma once

#include "base/base.h"

namespace io {

class Source {
  public:
    Source() {
    }
    virtual ~Source() {
    }

    void push(iovec io) {
      data_.push_back(io);
    }
    void push(char* data) {
      push(data, strlen(data));
    }
    void push(char* data, uint32 len) {
      iovec io = { 0 };
      io.iov_base = data;
      io.iov_len = len;
      push(io);
    }

    virtual const std::vector<iovec>& data() const {
      return data_;
    }

  protected:
    std::vector<iovec> data_;

  private:
    DISALLOW_COPY_AND_ASSIGN(Source);
};

}
