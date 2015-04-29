#ifndef ECHO_CLIENT_RESPONSER_H_
#define ECHO_CLIENT_RESPONSER_H_

#include "async/protocol.h"

namespace test {
using async::Connection;
using async::ProActorProtocol;

class EchoResponser : public ProActorProtocol::Scheluder {
  public:
    EchoResponser() {
    }
    virtual ~EchoResponser() {
    }

  private:
    virtual void dispatch(Connection* conn, io::InputStream* in_stream,
                          TimeStamp time_stamp) {
      // just ignore.
    }

    DISALLOW_COPY_AND_ASSIGN(EchoResponser);
};
}
#endif /* ECHO_CLIENT_RESPONSER_H_ */
