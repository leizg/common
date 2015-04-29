#ifndef ECHO_DISPATCHER_H_
#define ECHO_DISPATCHER_H_

#include "async/protocol.h"

namespace test {

class EchoDispatcher : public async::ProActorProtocol::Scheduler {
  public:
    EchoDispatcher() {
    }
    virtual ~EchoDispatcher() {
    }

  private:
    virtual void dispatch(async::Connection* conn, io::InputStream* in_stream,
                          TimeStamp time_stamp);

    DISALLOW_COPY_AND_ASSIGN(EchoDispatcher);
};
}
#endif /* ECHO_CLIENT_RESPONSER_H_ */
