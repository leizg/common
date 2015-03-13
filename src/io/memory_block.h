#ifndef MEMORY_BLOCK_H_
#define MEMORY_BLOCK_H_

#include "base/base.h"

namespace io {

class MemoryBlock {
  public:
    virtual ~MemoryBlock() {
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

    void retrieveRead(uint64 size) {
      rpos_ -= size;
    }
    void retrieveWrite(uint64 size) {
      wpos_ -= size;
    }
    void retrieveAll() {
      rpos_ = wpos_ = mem_;
    }

    void skipRead(uint64 size) {
      rpos_ += size;
    }
    void skipWrite(uint64 size) {
      wpos_ += size;
    }

    const char* peekR() const {
      return rpos_;
    }
    char* peekW() const {
      return wpos_;
    }

  protected:
    char* mem_;
    mutable char* rpos_;
    char* wpos_;
    char* end_;

    MemoryBlock()
        : mem_(nullptr), rpos_(nullptr), wpos_(nullptr), end_(nullptr) {
    }
    MemoryBlock(char* beg, char* end)
        : mem_(beg), rpos_(beg), wpos_(beg), end_(end) {
      DCHECK_NOTNULL(beg);
      DCHECK_NOTNULL(end);
    }
    MemoryBlock(char* data, uint32 len)
        : mem_(data), rpos_(data), wpos_(data), end_(data + len) {
      DCHECK_NOTNULL(data);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(MemoryBlock);
};

class ReadonlyBytesChunk : public MemoryBlock {
  public:
    ReadonlyBytesChunk(char* data, uint32 len, bool auto_release = true)
        : MemoryBlock(data, len), data_(data) {
      auto_release_ = auto_release;
      wpos_ = end_;
    }
    virtual ~ReadonlyBytesChunk() {
      if (auto_release_) {
        ::free(data_);
      }
    }

  private:
    char* data_;
    bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(ReadonlyBytesChunk);
};

class ReadonlyChunk : public MemoryBlock {
  public:
    explicit ReadonlyChunk(const MemoryBlock* block, bool auto_release = true)
        : MemoryBlock(block->rpos_, block->end_), block_(block) {
      wpos_ = end_;
      auto_release_ = auto_release;
    }
    virtual ~ReadonlyChunk() {
      if (auto_release_) {
        delete block_;
      }
    }

  private:
    const MemoryBlock* const block_;
    bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(ReadonlyChunk);
};

class ExternableChunk : public MemoryBlock {
  public:
    explicit ExternableChunk(uint64 size) {
      DCHECK_GT(size, 0);
      size = ALIGN(size);
      DCHECK_EQ(size % 4, 0);
      mem_ = (char*) ::malloc(size);
      end_ = mem_ + size;
      rpos_ = wpos_ = mem_;
    }
    virtual ~ExternableChunk() {
      ::free(mem_);
    }

    void ensureLeft(int len) {
      if (writeableSize() < len) {
        uint64 rn = readn(), wn = writen();
        if (rn >= len) {
          ::memcpy(mem_, rpos_, rn);
          rpos_ = mem_;
          wpos_ = mem_ + wn;
          return;
        }

        uint64 new_size = 512, min_size = len + wn;
        while (new_size <= min_size) {
          new_size *= 2;
        }
        char* data = (char*) ::malloc(new_size);
        DCHECK_NOTNULL(mem_);
        ::memcpy(data, rpos_, rn);
        ::free(mem_);
        mem_ = data;
        end_ = mem_ + new_size;
        rpos_ = mem_ + rn;
        wpos_ = mem_ + wn;
        DCHECK_GE(writeableSize(), len);
      }
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ExternableChunk);
};
}
#endif /* MEMORY_BLOCK_H_ */
