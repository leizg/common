#ifndef PROACTOR_PROTOCOL_H_
#define PROACTOR_PROTOCOL_H_

#include  "protocol.h" // Connection::Attr

namespace net {
class ProactorProtocol : public Protocol {
  public:
    virtual ~ProactorProtocol() {
    }

    enum IoStat {
      IO_START = 0, IO_HEADER, IO_DATA,
    };

    class Processor {
      public:
        virtual ~Processor() {
        }

        virtual void dispatch(Connection* conn, InputStream* input_buf,
                              const TimeStamp& time_stamp) = 0;
    };

    class Parser {
      public:
        virtual ~Parser() {
        }

        virtual uint32 headerLength() const = 0;
        virtual bool parse(Connection* const conn,
                           InputStream* const input_buf) const = 0;
    };

    class ErrorReporter {
      public:
        virtual ~ErrorReporter() {
        }

        virtual void report(Connection* conn) = 0;
    };

  protected:
    ProactorProtocol(Processor* processor, Parser* parser,
                     ErrorReporter* reporter =
                     NULL)
        : processor_(processor), parser_(parser), reporter_(reporter) {
      DCHECK_NOTNULL(processor);
      DCHECK_NOTNULL(parser);
    }

    virtual void handlePackage(Connection* conn, Connection::Attr* attr,
                               InputStream* input_buf) const {
    }

  private:
    scoped_ptr<Processor> processor_;
    scoped_ptr<Parser> parser_;

    scoped_ptr<ErrorReporter> reporter_;

    bool recvData(Connection* conn, Connection::Attr* attr,
                  uint32 data_len) const;
    void handleRead(Connection* conn, InputStream* input_buf,
                    const TimeStamp& time_stamp) const;

    DISALLOW_COPY_AND_ASSIGN(ProactorProtocol);
};
}

#endif /* PROTOCOL_H_ */
