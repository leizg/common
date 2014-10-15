#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include  "connection.h"

namespace io {
class InputBuf;
class Connection;

class Protocol {
 public:
  virtual ~Protocol() {
  }

  enum IoStat {
    IO_HEADER = 0,
    IO_DATA,
    IO_END,
  };

  class Processor {
   public:
    virtual ~Processor() {
    }

    virtual void Dispatch(Connection* conn, InputBuf* input_buf,
                          const TimeStamp& time_stamp) = 0;
  };

  virtual Connection::Attr* NewConnectionAttr() const = 0;

  // not thread safe.
  // should be called as soon as possible.
  void SetProcessor(Processor* p) {
    CHECK(processor_.get() == NULL);
    processor_.reset(p);
  }

  void handleRead(Connection* conn, InputBuf* input_buf,
                  const TimeStamp& time_stamp) const;

 protected:
  Protocol() {
  }

  virtual bool ParseHeader(Connection* conn, InputBuf* input_buf) const = 0;

 private:
  scoped_ptr<Processor> processor_;

  // return true iff all data be received.
  bool RecvData(Connection* conn, InputBuf* input_buf) const;
  int32 GetNextSegmentLength(Connection* conn, Connection::Attr* attr,
                             InputBuf* input_buf) const;

  DISALLOW_COPY_AND_ASSIGN(Protocol);
};
}

#endif /* PROTOCOL_H_ */
