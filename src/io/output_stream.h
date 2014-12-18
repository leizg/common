#ifndef OUTPUT_STREAM_H_
#define OUTPUT_STREAM_H_

#include "memory_block.h"

namespace io {

class OutputStream {
  public:
    explicit OutputStream(uint64 size = 512) {
      block_.reset(new ExternableChunk(size));
    }
    virtual ~OutputStream() {
    }

    void Next(char** buf, uint64* len);
    void Backup(uint64 len) {
      block_->backup(len);
    }

    uint64 ByteCount() const {
      return block_->writen();
    }

    void ensureLeft(int64 size) {
      block_->ensureLeft(size);
    }

    char* peek() {
      return block_->peekW();
    }

  private:
    scoped_ref<ExternableChunk> block_;

    DISALLOW_COPY_AND_ASSIGN(OutputStream);
};

inline void OutputStream::Next(char** buf, uint64* len) {
  block_->ensureLeft(*len);
  *buf = block_->peekW();
  block_->skip(*len);
}
}
#endif /* OUTPUT_STREAM_H_ */
