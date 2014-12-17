#ifndef ECHO_DISPATCHER_H_
#define ECHO_DISPATCHER_H_

#include "io/protocol.h"

namespace test {

class EchoDispatcher : public io::Protocol::Processor {
  public:
    EchoDispatcher() {
    }
    virtual ~EchoDispatcher() {
    }

  private:
    virtual void dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(EchoDispatcher);
};
}
#endif /* ECHO_CLIENT_RESPONSER_H_ */
