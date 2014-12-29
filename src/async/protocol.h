#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "connection.h" // Connection::Attr

namespace io {
class InputStream;
}

namespace aync {

class Protocol {
  public:
    virtual ~Protocol() {
    }

    virtual Connection::Attr* NewConnectionAttr() const = 0;

    typedef io::InputStream InputStream;
    virtual void handleRead(Connection* conn, InputStream* input_buf,
                            const TimeStamp& time_stamp) const = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(Protocol);
};
}
#endif /* PROTOCOL_H_ */
