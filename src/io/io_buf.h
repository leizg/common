#ifndef IO_BUF_H_
#define IO_BUF_H_

#include "base/base.h"

namespace io {

class OutputObject {
 public:
  virtual ~OutputObject() {
  }

  virtual int Send(int fd, int32* err_no) = 0;
};

}

#endif /* IO_BUF_H_ */
