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

    void retrieve(uint64 size) {
      rpos_ -= size;
    }
    void retrieveAll() {
      rpos_ = wpos_ = mem_;
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
        : mem_(NULL), rpos_(NULL), wpos_(NULL), end_(NULL) {
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
        : MemoryBlock(data, len), data_(data), auto_release_(auto_release) {
    }
    virtual ~ReadonlyBytesChunk() {
      if (auto_release_) {
        ::free(const_cast<char*>(data_));
      }
    }

  private:
    const char* const data_;
    const bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(ReadonlyBytesChunk);
};

class ReadonlyBlockChunk : public MemoryBlock {
  public:
    ReadonlyBlockChunk(const MemoryBlock* block, bool auto_release = true)
        : MemoryBlock(block->rpos_, block->end_), block_(block), auto_release_(
            auto_release) {
    }
    virtual ~ReadonlyBlockChunk() {
      if (auto_release_) {
        delete block_;
      }
    }

  private:
    const MemoryBlock* const block_;
    const bool auto_release_;

    DISALLOW_COPY_AND_ASSIGN(ReadonlyBlockChunk);
};

class ExternableChunk : public MemoryBlock {
  public:
    explicit ExternableChunk(uint64 size) {
      CHECK_GT(size, 0);
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

        uint64 new_size = capacity() * 2;
        uint64 free_size = new_size - wn;
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

    void skip(uint64 size) {
      wpos_ += size;
      DCHECK_LE(wpos_, end_);
    }
    void backup(uint64 size) {
      wpos_ -= size;
      DCHECK_LE(rpos_, wpos_);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ExternableChunk);
};
}
#endif /* MEMORY_BLOCK_H_ */
