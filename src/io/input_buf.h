#ifndef INPUT_BUF_H_
#define INPUT_BUF_H_

#include "memory_block.h"
#include "include/zero_copy_stream.h"

namespace io {

class InputBuf : public MemoryBlock, public ZeroCopyInputStream {
 public:
  InputBuf(uint32 size) {
    size = ALIGN(size);
    mem_ = ::malloc(size);
    end_ = mem_ + size;
    rpos_ = wpos_ = mem_;
  }
  virtual ~InputBuf() {
    ::free(mem_);
  }

  // by ZeroCopyInputStream.
  virtual int32 Next(char** buf, uint32* len);

  virtual void Skip(uint32 len) {
    rpos_ += len;
    CHECK_LE(rpos_, wpos_);
  }
  virtual void Backup(uint32 len) {
    rpos_ -= len;
    CHECK_LE(mem_, rpos_);
  }

  int32 ReadFd(int fd, uint32 len, int32* err_no);

 private:
  void EnsureLeft(uint32 len);

  DISALLOW_COPY_AND_ASSIGN(InputBuf);
};
}

#endif /* INPUT_BUF_H_ */
