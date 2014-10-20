#ifndef INPUT_BUF_H_
#define INPUT_BUF_H_

#include "memory_block.h"

namespace io {

class InputBuf {
  public:
    explicit InputBuf(uint32 size)
        : index_(0), readn_(0) {
      MemoryBlock* blk = new MemoryBlock(size);
      blocks_.push_back(blk);
    }
    virtual ~InputBuf() {
      STLUnRef(&blocks_);
    }

    // return false iif no data can be read.
    // buf and len must be null.
    bool Next(const char** buf, int* len);

    void Skip(uint32 len);
    void Backup(uint32 len);

    // return the total number of bytes read since this object was created.
    int ByteCount() const {
      return readn_;
    }

    int32 ReadFd(int fd, uint32 len, int32* err_no);

  private:
    int32 index_;
    uint32 readn_;
    std::vector<MemoryBlock*> blocks_;

    MemoryBlock* FindDataBlock();

    int32 ReadFdInternal(MemoryBlock*block, int fd, uint32 len, int32* err_no);

    DISALLOW_COPY_AND_ASSIGN(InputBuf);
};
}

#endif /* INPUT_BUF_H_ */
