#ifndef OUTPUT_BUF_H_
#define OUTPUT_BUF_H_

#include "memory_block.h"

namespace io {

class OutputBuf {
  public:
    explicit OutputBuf(uint32 size)
        : index_(0), size_(0) {
      MemoryBlock* block = new MemoryBlock(size);
      blocks_.push_back(block);
    }
    virtual ~OutputBuf() {
      STLClear(&blocks_);
    }

    void Next(char** buf, int* len);
    void Backup(uint32 len);
    uint64 ByteCount() const {
      return size_;
    }

    void data(std::vector<iovec>* iov) const {
      for (uint32 i = 0; i < blocks_.size(); ++i) {
        MemoryBlock* block = blocks_[i];
        iovec io;
        io.iov_base = block->rpos_;
        io.iov_len = block->size();
        iov->push_back(io);
      }
    }

  private:
    uint32 index_;
    uint64 size_;
    std::vector<MemoryBlock*> blocks_;

    DISALLOW_COPY_AND_ASSIGN(OutputBuf);
};
}

#endif /* OUTPUT_BUF_H_ */
