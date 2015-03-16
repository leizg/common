#ifndef ECHO_CLIENT_RESPONSER_H_
#define ECHO_CLIENT_RESPONSER_H_

#include "async/protocol.h"

namespace test {

class EchoResponser : public async::ProReactorProtocol::Scheluder {
  public:
    EchoResponser() {
    }
    virtual ~EchoResponser() {
    }

  private:
    virtual void dispatch(async::Connection* conn, io::InputStream* in_stream,
                          TimeStamp time_stamp) {
      // just ignore.
    }

    DISALLOW_COPY_AND_ASSIGN(EchoResponser);
};
}
#endif /* ECHO_CLIENT_RESPONSER_H_ */
