#ifndef ECHO_PROTOCOL_H_
#define ECHO_PROTOCOL_H_

#include "async/protocol.h"

namespace test {
class EchoDispatcher;

class EchoProtocol : public async::ProReactorProtocol {
  public:
    explicit EchoProtocol(async::ProReactorProtocol::Scheluder* scheluder);
    virtual ~EchoProtocol();

  private:
    class EchoParser : public async::ProReactorProtocol::Parser {
      public:
        EchoParser() {
        }
        virtual ~EchoParser() {
        }

      private:
        virtual uint32 headerLength() const {
          return sizeof(uint32);
        }

        virtual bool parseHeader(async::Connection* conn) const;

        DISALLOW_COPY_AND_ASSIGN(EchoParser);
    };

    virtual async::Connection::UserData* NewConnectionData() const {
      return new UserData;
    }

    DISALLOW_COPY_AND_ASSIGN(EchoProtocol);
};

}

#endif /* ECHO_PROTOCOL_H_ */
