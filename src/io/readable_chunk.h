#ifndef READABLE_CHUNK_H_
#define READABLE_CHUNK_H_

#include "memory_block.h"

namespace io {

class ReadableAbstruct : public RefCounted {
  public:
    virtual ~ReadableAbstruct() {
    }

    const std::vector<iovec>& iov() const {
      return iov_;
    }

    virtual void buildData() = 0;

  protected:
    std::vector<iovec> iov_;

    void attatchData(const char* data, uint32 len) {
      iovec io;
      io.iov_base = const_cast<char*>(data);
      io.iov_len = len;
      iov_.push_back(io);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ReadableAbstruct);
};

class ReadalbeBytes : public ReadableAbstruct {
  public:
    ReadalbeBytes(char* data, uint32 len, bool auto_release = true)
        : data_(data), len_(len), auto_release_(auto_release) {
    }
    virtual ~ReadalbeBytes() {
    }

    virtual void buildData() {
      if (iov_.empty()) {
        attatchData(data_, len_);
      }
    }

  private:
    const char* data_;
    const uint32 len_;
    const bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(ReadalbeBytes);
};

class ReadFdChunk : public ReadableAbstruct, public ExternableChunk {
  public:
    explicit ReadFdChunk(uint64 size) {
    }
    virtual ~ReadFdChunk() {
    }

    virtual void buildData() {
      if (iov_.empty()) {
        attatchData(rpos_, readableSize());
      }
    }

    int32 readFd(int fd, uint32 len, int32* err_no);

  private:
    DISALLOW_COPY_AND_ASSIGN(ReadFdChunk);
};

}
#endif /* READABLE_CHUNK_H_ */
