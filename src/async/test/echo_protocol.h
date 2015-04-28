#ifndef ECHO_PROTOCOL_H_
#define ECHO_PROTOCOL_H_

#include "async/protocol.h"

namespace test {
using async::Connection;
using async::ProReactorProtocol;

class EchoProtocol : public ProReactorProtocol {
  public:
    explicit EchoProtocol(Scheluder* scheluder);
    virtual ~EchoProtocol();

  private:
    class EchoParser : public Parser {
      public:
        EchoParser() {
        }
        virtual ~EchoParser() {
        }

      private:
        virtual bool parseHeader(Connection* conn) const;
        virtual uint32 headerLength() const {
          return sizeof(uint32);
        }

        DISALLOW_COPY_AND_ASSIGN(EchoParser);
    };

    virtual UserData* NewConnectionData() const {
      return new UserData;
    }

    DISALLOW_COPY_AND_ASSIGN(EchoProtocol);
};

}

#endif /* ECHO_PROTOCOL_H_ */
