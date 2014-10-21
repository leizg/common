#include "input_buf.h"

namespace io {

MemoryBlock* InputBuf::FindDataBlock() {
  MemoryBlock* block;
  while (true) {
    block = blocks_[index_];
    if (block->left() != 0) {
      break;
    }
    if (index_ == blocks_.size()) {
      return NULL;
    }
    index_++;
  }

  return block;
}

bool InputBuf::Next(const char** buf, int* len) {
  MemoryBlock* block = FindDataBlock();
  if (block != NULL) {
    *buf = block->rpos_;
    *len = std::min(*len, block->left());
    block->rpos_ += *len;
    readn_ += *len;
    CHECK_LE(block->rpos_, block->wpos_);
    return true;
  }

  return false;
}

void InputBuf::Skip(uint32 len) {
  uint32 left = len;
  while (left > 0) {
    MemoryBlock* block = blocks_[index_];
    if (block->size() >= left) {
      block->rpos_ += left;
      readn_ += left;
      CHECK_LE(block->rpos_, block->wpos_);
      return;
    }

    left -= block->size();
    readn_ += block->size();
    block->rpos_ += block->size();
    if (index_ == blocks_.size()) {
      break;
    }
    index_++;
  }
}

void InputBuf::Backup(uint32 len) {
  while (len > 0) {
    MemoryBlock* block = blocks_[index_];
    if (block->readn() >= len) {
      block->rpos_ -= len;
      readn_ -= len;
      return;
    }

    len -= block->readn();
    readn_ -= block->readn();
    block->rpos_ -= block->readn();

    if (index_ == 0) return;
    index_--;
  }
}

int32 InputBuf::ReadFdInternal(MemoryBlock*block, int fd, uint32 total_len,
                               int32* err_no) {
  uint32 left_len = total_len;
  while (left_len > 0) {
    int n = ::recv(fd, block->wpos_, left_len, 0);
    if (n == 0) return 0;
    else if (n == -1) {
      switch (errno) {
        case EINTR:
          continue;
          break;

        case EAGAIN:
        case EWOULDBLOCK:
          return total_len - left_len;

        default:
          if (err_no != NULL) {
            *err_no = errno;
          }
          PLOG(WARNING)<<"recv error";
          return -1;
        }  // end switch.
      }

    left_len -= n;
    block->wpos_ += n;
  }

  return total_len - left_len;
}

int32 InputBuf::ReadFd(int fd, uint32 total_len, int32* err_no) {
  CHECK(!blocks_.empty());
  int left = total_len;
  while (left > 0) {
    MemoryBlock* block = *blocks_.rbegin();
    if (block->left() == 0) {
      block = new MemoryBlock(left);
      blocks_.push_back(block);
      continue;
    }

    uint32 len = std::min(left, block->left());
    int32 ret = ReadFdInternal(block, fd, len, err_no);
    if (ret == -1 || ret == 0) {  // read error.
      return ret;
    }
    left -= ret;
  }

  return total_len - left;
}

}
