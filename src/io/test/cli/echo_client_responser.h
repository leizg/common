#ifndef ECHO_CLIENT_RESPONSER_H_
#define ECHO_CLIENT_RESPONSER_H_

#include "io/protocol.h"

namespace test {

class EchoResponser : public io::Protocol::Processor {
  public:
    EchoResponser() {
    }
    virtual ~EchoResponser() {
    }

  private:
    virtual void dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp) {
      // just ignore.
    }

    DISALLOW_COPY_AND_ASSIGN(EchoResponser);
};
}
#endif /* ECHO_CLIENT_RESPONSER_H_ */
