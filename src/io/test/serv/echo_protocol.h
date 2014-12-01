#ifndef ECHO_PROTOCOL_H_
#define ECHO_PROTOCOL_H_

#include "io/protocol.h"

namespace test {

class EchoProtocol : public io::Protocol {
  private:
    class EchoProcessor : public Processor {
      public:
        EchoProcessor() {
        }
        virtual ~EchoProcessor() {
        }

      private:
        virtual void dispatch(io::Connection* conn, io::InputBuf* input_buf,
                              const TimeStamp& time_stamp);

        DISALLOW_COPY_AND_ASSIGN(EchoProcessor);
    };

    class EchoParser : public Parser {
      public:
        EchoParser() {
        }
        virtual ~EchoParser() {
        }

      private:

        virtual uint32 headerLength() const {
          return sizeof(uint32);
        }
        virtual bool parse(io::Connection* const conn,
                           io::InputBuf* const input_buf) const;

        DISALLOW_COPY_AND_ASSIGN(EchoParser);
    };

    class EchoAttr : public io::Connection::Attr {
      public:
        EchoAttr() {
          Init();
        }
        virtual ~EchoAttr() {
        }

      private:
        virtual void Init();

        DISALLOW_COPY_AND_ASSIGN(EchoAttr);
    };

  public:
    EchoProtocol()
        : io::Protocol(new EchoProcessor, new EchoParser) {
    }
    virtual ~EchoProtocol() {
    }

  private:
    virtual io::Connection::Attr* NewConnectionAttr() const {
      return new EchoAttr;
    }

    DISALLOW_COPY_AND_ASSIGN(EchoProtocol);
};

}

#endif /* ECHO_PROTOCOL_H_ */
