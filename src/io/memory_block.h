#ifndef MEMORY_BLOCK_H_
#define MEMORY_BLOCK_H_

#include "base/base.h"

namespace io {

class MemoryBlock {
 public:
  MemoryBlock() {
    mem_ = rpos_ = wpos_ = end_ = NULL;
  }
  virtual ~MemoryBlock() {
  }

  inline int32 capacity() const {
    return end_ - mem_;
  }
  inline uint32 size() const {
    return wpos_ - rpos_;
  }
  inline uint32 left() const {
    return end_ - wpos_;
  }
  inline uint32 empty() const {
    return NULL == mem_;
  }

  inline char* peekR() const {
    return rpos_;
  }
  inline char* peekW() const {
    return wpos_;
  }

  void review() {
    rpos_ = wpos_ = mem_;
  }

  inline uint32 readn() const {
    return rpos_ - mem_;
  }
  inline uint32 writen() const {
    return wpos_ - mem_;
  }

 protected:
  char* mem_;
  char* rpos_;
  char* wpos_;
  char* end_;

 private:
  DISALLOW_COPY_AND_ASSIGN(MemoryBlock);
}
;
}

#endif /* MEMORY_BLOCK_H_ */
