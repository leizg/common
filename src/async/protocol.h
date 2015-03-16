#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "connection.h"

namespace io {
class InputStream;
class ExternableChunk;
class ConcatenaterSource;
}

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

    class Parser {
      public:
        virtual ~Parser() {
        }

        virtual uint32 headerLength() const = 0;
        virtual bool parseHeader(Connection* conn) const = 0;
    };

    class Scheluder {
      public:
        virtual ~Scheluder() {
        }

        virtual void dispatch(Connection* conn, io::InputStream* in_stream,
                              TimeStamp time_stamp) = 0;
    };

    enum IoStat {
      IO_START = 0, IO_HEADER, IO_BODY, IO_END,
    };

    struct UserData : public Connection::UserData {
        UserData();
        virtual ~UserData();

        const char* peekHeader() const;
        io::InputStream* releaseStream();
        void newPackage();

        bool is_last;
        IoStat io_stat;
        uint32 pending_size;
        scoped_ptr<io::ExternableChunk> chunk;
        scoped_ptr<io::ConcatenaterSource> src;

        // todo: out queue.
    };

  protected:
    ProReactorProtocol(Parser* parser, Scheluder* scheluder,
                       ErrorReporter* reporter = NULL)
        : parser_(parser), scheluder_(scheluder), reporter_(reporter) {
      DCHECK_NOTNULL(parser);
      DCHECK_NOTNULL(scheluder);
    }

  private:
    scoped_ptr<Parser> parser_;
    scoped_ptr<Scheluder> scheluder_;
    scoped_ptr<ErrorReporter> reporter_;

    virtual void handleRead(Connection* conn, TimeStamp time_stamp);
    virtual void handleWrite(Connection* conn, TimeStamp time_stamp);
    virtual void handleError(Connection* conn);
    virtual void handleClose(Connection* conn);

    bool recvData(Connection* conn, UserData* u, uint32 data_len);
    bool RecvPending(Connection* conn, UserData* ud);

    DISALLOW_COPY_AND_ASSIGN(ProReactorProtocol);
};

}
#endif /* PROTOCOL_H_ */
