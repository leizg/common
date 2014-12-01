#ifndef MEMORY_BLOCK_H_
#define MEMORY_BLOCK_H_

#include "base/base.h"

namespace io {

class MemoryBlock : public RefCounted {
  public:
    explicit MemoryBlock(int size) {
      CHECK_GT(size, 0);
      size = ALIGN(size);
      DCHECK_EQ(size % 4, 0);
      mem_ = (char*) ::malloc(size);
      end_ = mem_ + size;
      rpos_ = wpos_ = mem_;
    }

    int capacity() const {
      return end_ - mem_;
    }

    int readn() const {
      return rpos_ - mem_;
    }
    int writen() const {
      return wpos_ - mem_;
    }

    int readableSize() const {
      return wpos_ - rpos_;
    }
    int writeableSize() const {
      return end_ - wpos_;
    }

    void review() {
      rpos_ = wpos_ = mem_;
    }

    void ensureLeft(int len);

    char* peekR() {
      return rpos_;
    }
    char* peekW() {
      return wpos_;
    }

  private:
    virtual ~MemoryBlock() {
      ::free(mem_);
    }

    char* mem_;
    char* rpos_;
    char* wpos_;
    char* end_;

    friend class InputBuf;
    friend class OutputBuf;

    DISALLOW_COPY_AND_ASSIGN(MemoryBlock);
};

inline void MemoryBlock::ensureLeft(int len) {
  if (writeableSize() < len) {
    int rn = readn(), wn = writen();
    int new_size = capacity() * 2;
    int free_size = new_size - wn;
    if (free_size < len) {
      new_size += len - free_size;
    }

    mem_ = (char*) ::realloc(mem_, new_size);
    end_ = mem_ + new_size;
    rpos_ = mem_ + rn;
    wpos_ = mem_ + wn;

    DCHECK_GE(writeableSize(), len);
  }
}

}

#endif /* MEMORY_BLOCK_H_ */
