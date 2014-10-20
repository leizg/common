#include "output_buf.h"

namespace io {

void OutputBuf::Next(char** buf, int* len) {
  MemoryBlock* blk = blocks_[index_];
  if (blk->left() != 0) {
    *len = std::min(*len, blk->left());
    size_ += *len;
    blk->wpos_ += *len;
    return;
  }

  blk = new MemoryBlock(*len);
  size_ += *len;
  blk->wpos_ += *len;
}

void OutputBuf::Backup(uint32 len) {
  while (len != 0) {
    MemoryBlock* blk = blocks_[index_];
    if (blk->writen() >= len) {
      blk->wpos_ -= len;
      size_ -= len;
      return;
    }

    len -= blk->writen();
    blk->wpos_ -= blk->writen();
    size_ -= blk->writen();
    index_--;
  }
}

}
