#ifndef ZERO_COPY_STREAM_H_
#define ZERO_COPY_STREAM_H_

#include "io/input_buf.h"
#include "io/output_buf.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace rpc {

//todo:
class InputStream : public ::google::protobuf::io::ZeroCopyInputStream {
  public:
    InputStream(uint32 len)
        : buf_(len) {
    }
    virtual ~InputStream() {
    }

    virtual bool Next(const void** buf, int* len) {
      return buf_.Next((const char**) buf, len);
    }

    virtual bool Skip(int len) {
      return buf_.Skip(len);
    }
    virtual void BackUp(int len) {
      buf_.Backup(len);
    }
    virtual int64 ByteCount(uint32 len) const {
      return buf_.ByteCount();
    }

  private:
    io::InputBuf buf_;

    DISALLOW_COPY_AND_ASSIGN(InputStream);
};

class OutputStream : public ::google::protobuf::io::ZeroCopyOutputStream {
  public:
    virtual ~OutputStream() {
    }

    virtual bool Next(void** data, int* size) {
      return buf_.Next((char**) data, size);
    }
    virtual void BackUp(int len) {
      buf_.Backup(len);
    }
    virtual uint64 ByteCount() const {
      return buf_.ByteCount();
    }

  private:
    io::OutputBuf buf_;

    DISALLOW_COPY_AND_ASSIGN(OutputStream);
};

}
#endif /* ZERO_COPY_STREAM_H_ */
