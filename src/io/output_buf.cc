#include "output_buf.h"

namespace io {

void OutputBuf::Next(char** buf, int* len) {
  DCHECK_GT(*len, 0);
  block_->ensureLeft(*len);
  *buf = block_->wpos_;
  block_->wpos_ += *len;
  DCHECK_LE(block_->wpos_, block_->end_);
}

void OutputBuf::Backup(uint32 len) {
  DCHECK_GE(block_->writeableSize(), len);
  block_->wpos_ -= len;
}

}
