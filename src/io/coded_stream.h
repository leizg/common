#ifndef CODED_STREAM_H_
#define CODED_STREAM_H_

#include "base/base.h"

namespace io {
class InputStream;
class CodedInputStream {
  public:
    explicit CodedInputStream(InputStream* stream, bool auto_release = false)
        : stream_(stream), auto_release_(auto_release) {
      DCHECK_NOTNULL(stream);
    }
    ~CodedInputStream();

    bool readInt32(uint32* val) const {
      return readBytes(static_cast<char*>(val), sizeof(uint32));
    }
    bool readInt64(uint64* val) const {
      return readBytes(static_cast<char*>(val), sizeof(uint64));
    }
    bool readBytes(char* buf, uint64 len) const;
    bool readString(std::string* str, uint32 len) const {
      str->resize(len);
      return readBytes(&(*str)[0], len);
    }

  private:
    const InputStream* const stream_;
    const bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(CodedInputStream);
};

class OutputStream;
class CodedOutputStream {
  public:
    CodedOutputStream(OutputStream* stream)
        : stream_(stream) {
    }
    ~CodedOutputStream() {
    }

    void writeInt32(uint32 val) {
      writeBytes(static_cast<char*>(&val), sizeof(val));
    }
    void writeInt64(uint64 val) {
      writeBytes(static_cast<char*>(&val), sizeof(val));
    }
    void writeBytes(const char* data, uint64 len);
    void writeString(const std::string& str) {
      writeBytes(str.data(), str.size());
    }

  private:
    OutputStream* stream_;

    DISALLOW_COPY_AND_ASSIGN(CodedOutputStream);
};
}
#endif /* CODED_STREAM_H_ */
