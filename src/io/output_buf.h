#ifndef OUTPUT_BUF_H_
#define OUTPUT_BUF_H_

#include "memory_block.h"

namespace io {

class OutputBuf {
  public:
    explicit OutputBuf(uint32 size) {
      block_.reset(new MemoryBlock(size));
    }
    virtual ~OutputBuf() {
    }

    void Next(char** buf, int* len);
    void Backup(uint32 len);

    uint64 ByteCount() const {
      return block_->writen();
    }

    void ensureLeft(int64 size) {
      block_->ensureLeft(size);
    }

  private:
    scoped_ref<MemoryBlock> block_;

    DISALLOW_COPY_AND_ASSIGN(OutputBuf);
};
}

#endif /* OUTPUT_BUF_H_ */
