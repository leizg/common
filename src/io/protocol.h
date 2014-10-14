#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "base/base.h"

namespace io {
class InputBuf;
class Connection;

class Protocol {
 public:
  class Processor {
   public:
    virtual ~Processor() {
    }

    virtual void Dispatch(Connection* conn, InputBuf* input_buf,
                          const TimeStamp& time_stamp) = 0;
  };

  virtual ~Protocol() {
  }

  // not thread safe.
  void SetProcessor(Processor* p) {
    CHECK(processor_.get() == NULL);
    processor_.reset(p);
  }

  void handleRead(Connection* conn, InputBuf* input_buf,
                  const TimeStamp& time_stamp);

 protected:
  Protocol() {
  }

  virtual bool ParseHeader(Connection* conn, InputBuf* input_buf) const = 0;

 private:
  scoped_ptr<Processor> processor_;

  DISALLOW_COPY_AND_ASSIGN(Protocol);
};
}

#endif /* PROTOCOL_H_ */
