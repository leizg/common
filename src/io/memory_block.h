#ifndef MEMORY_BLOCK_H_
#define MEMORY_BLOCK_H_

#include "base/base.h"

namespace io {

class MemoryBlock : public RefCounted {
  public:
    explicit MemoryBlock(int size) {
      size = ALIGN(size);
      mem_ = ::malloc(size);
      end_ = mem_ + size;
      rpos_ = wpos_ = mem_;
    }

    inline int capacity() const {
      return end_ - mem_;
    }
    inline int size() const {
      return wpos_ - rpos_;
    }
    inline int left() const {
      return end_ - wpos_;
    }
    inline int empty() const {
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

    inline int readn() const {
      return rpos_ - mem_;
    }
    inline int writen() const {
      return wpos_ - mem_;
    }

    void EnsureLeft(int len);

  protected:
    virtual ~MemoryBlock() {
      ::free(mem_);
    }

  private:
    char* mem_;
    char* rpos_;
    char* wpos_;
    char* end_;

    friend class InputBuf;
    friend class OutputBuf;

    DISALLOW_COPY_AND_ASSIGN(MemoryBlock);
};

inline void MemoryBlock::EnsureLeft(int len) {
  if (left() < len) {
    int rn = readn(), wn = writen();
    int new_size = capacity() * 2;
    int left_size = new_size - wn;
    if (left_size < len) {
      new_size += len - left_size;
    }

    mem_ = ::realloc(mem_, new_size);
    end_ = mem_ + new_size;
    rpos_ = mem_ + rn;
    wpos_ = mem_ + wn;
  }
}

}

#endif /* MEMORY_BLOCK_H_ */
