#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "connection.h" // Connection::Attr

namespace io {
class ReadableAbstruct;
}

namespace async {

class Protocol {
  public:
    virtual ~Protocol() {
    }

    enum IoStat {
      IO_START = 0, IO_HEADER, IO_DATA,
    };

    class ErrorReporter {
      public:
        virtual ~ErrorReporter() {
        }

        virtual void report(Connection* conn) = 0;
    };

    class Scheluder {
      public:
        virtual ~Scheluder() {
        }

        virtual void dispatch(Connection* conn, io::ReadableAbstruct* input_buf,
                              const TimeStamp& time_stamp) = 0;
    };

    virtual Connection::Attr* NewConnectionAttr() const = 0;

    void handleRead(Connection* conn, io::ReadableAbstruct* input_buf,
                    const TimeStamp& time_stamp) const;

  protected:
    explicit Protocol(Scheluder* scheluder)
        : scheluder_(scheluder) {
      DCHECK_NOTNULL(scheluder);
    }

  private:
    scoped_ptr<Scheluder> scheluder_;

    bool Protocol::recvData(Connection* conn, Connection::Attr* attr,
                            uint32 data_len) const;

    DISALLOW_COPY_AND_ASSIGN(Protocol);
};
}
#endif /* PROTOCOL_H_ */
