#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "connection.h"

namespace async {

class Protocol {
  public:
    virtual ~Protocol() {
    }

    virtual Connection::UserData* NewConnectionData() const = 0;

    virtual void handleRead(Connection* conn, TimeStamp time_stamp) = 0;
    virtual void handleWrite(Connection* conn, TimeStamp time_stamp) = 0;
    virtual void handleError(Connection* conn) = 0;
    virtual void handleClose(Connection* conn) = 0;

  protected:
    Protocol() {
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Protocol);
};

class ProReactorProtocol : public Protocol {
  public:
    virtual ~ProReactorProtocol() {
    }

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

        virtual void dispatch(Connection* conn, TimeStamp time_stamp) = 0;
    };

    enum IoStat {
      IO_START = 0, IO_HEADER, IO_DATA, IO_END,
    };

    struct UserData : public Connection::UserData {
        UserData()
            : is_last(true) {
          io_stat = IO_START;
          pending_size = 0;
        }

        bool is_last;
        IoStat io_stat;
        uint32 pending_size;
    };

  protected:
    ProReactorProtocol(Scheluder* scheluder, ErrorReporter* reporter)
        : scheluder_(scheluder), reporter_(reporter) {
      DCHECK_NOTNULL(scheluder);
    }

    virtual uint32 headerLength() const = 0;
    virtual bool parseHeader(Connection* conn) const;

  private:
    scoped_ptr<Scheluder> scheluder_;
    scoped_ptr<ErrorReporter> reporter_;

    virtual void handleRead(Connection* conn, TimeStamp time_stamp);
    virtual void handleWrite(Connection* conn, TimeStamp time_stamp);
    virtual void handleError(Connection* conn);
    virtual void handleClose(Connection* conn);

    DISALLOW_COPY_AND_ASSIGN(ProReactorProtocol);
};

}
#endif /* PROTOCOL_H_ */
