#ifndef OUTPUT_BUF_H_
#define OUTPUT_BUF_H_

#include "memory_block.h"
#include "include/zero_copy_stream.h"

namespace io {

class OutputBuf : public MemoryBlock, public ZeroCopyOutputStream {
 public:
  OutputBuf();
  virtual ~OutputBuf();

  // not change *len
  virtual void Next(char** buf, uint32* len) {
    EnsureLeft(*len);
    *buf = wpos_;
    wpos_ += *len;
    CHECK_LE(wpos_, end_);
  }

  virtual void Backup(uint32 len) {
    wpos_ -= len;
    CHECK_LE(rpos_, wpos_);
  }

 private:
  void EnsureLeft(uint32 len) {
    if (left() < len) {
      uint32 rn = readn(), wn = writen();
      uint32 new_size = capacity() * 2;
      uint32 left_size = new_size - wn;
      if (left_size < len) {
        new_size += len - left_size;
      }

      mem_ = ::realloc(mem_, new_size);
      end_ = mem_ + new_size;
      rpos_ = mem_ + rn;
      wpos_ = mem_ + wn;
    }
  }

  DISALLOW_COPY_AND_ASSIGN(OutputBuf);
};
}

#endif /* OUTPUT_BUF_H_ */
