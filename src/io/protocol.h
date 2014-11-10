#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "connection.h"

namespace io {
class InputBuf;

class Protocol {
  public:
    virtual ~Protocol() {
    }

    enum IoStat {
      IO_START = 0, IO_HEADER, IO_DATA,
    };

    class Processor {
      public:
        virtual ~Processor() {
        }

        virtual void dispatch(Connection* conn, InputBuf* input_buf,
                              const TimeStamp& time_stamp) = 0;
    };

    class Parser {
      public:
        virtual ~Parser() {
        }

        virtual uint32 headerLength() const = 0;
        virtual bool parse(Connection* const conn,
                                 InputBuf* const input_buf) const = 0;
    };

    class ErrorReporter {
      public:
        virtual ~ErrorReporter() {
        }

        virtual void report(Connection* conn) = 0;
    };

    virtual Connection::Attr* NewConnectionAttr() const = 0;

    void handleRead(Connection* conn, InputBuf* input_buf,
                    const TimeStamp& time_stamp) const;

  protected:
    Protocol(Processor* processor, Parser* parser, ErrorReporter* reporter =
    NULL)
        : processor_(processor), parser_(parser), reporter_(reporter) {
      DCHECK_NOTNULL(processor);
      DCHECK_NOTNULL(parser);
    }

    virtual void handlePackage(Connection* conn, Connection::Attr* attr,
                               InputBuf* input_buf) const {
    }

  private:
    scoped_ptr<Processor> processor_;
    scoped_ptr<Parser> parser_;

    scoped_ptr<ErrorReporter> reporter_;

    bool recvData(Connection* conn, Connection::Attr* attr,
                  uint32 data_len) const;

    DISALLOW_COPY_AND_ASSIGN(Protocol);
};
}

#endif /* PROTOCOL_H_ */
