#ifndef INPUT_BUF_H_
#define INPUT_BUF_H_

#include "memory_block.h"

namespace io {

class InputBuf {
  public:
    explicit InputBuf(uint32 size) {
      block_.reset(new MemoryBlock(size));
    }
    virtual ~InputBuf() {
    }

    // return false iif no data can be read.
    // buf and len must be null.
    bool Next(const char** buf, int* len);

    void Skip(uint32 len);
    void Backup(uint32 len);

    // return the total number of bytes read since this object was created.
    int ByteCount() {
      return block_->readn();
    }

    int32 ReadFd(int fd, uint32 len, int32* err_no);

  private:
    scoped_ref<MemoryBlock> block_;

    DISALLOW_COPY_AND_ASSIGN(InputBuf);
};
}
#endif /* INPUT_BUF_H_ */
