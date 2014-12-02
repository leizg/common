#include "input_buf.h"

namespace io {

bool InputBuf::Next(const char** buf, int* len) {
  if (block_->readableSize() == 0) {
    *buf = NULL;
    *len = 0;
    return false;
  }

  *buf = block_->rpos_;
  *len = std::min(*len, block_->readableSize());
  block_->rpos_ += *len;
  DCHECK_LE(block_->rpos_, block_->wpos_);

  return true;
}

void InputBuf::Skip(uint32 len) {
  DCHECK_GE(block_->readableSize(), len);
  block_->rpos_ += len;
  DCHECK_LE(block_->rpos_, block_->wpos_);
}

void InputBuf::Backup(uint32 len) {
  DCHECK_GE(block_->readn(), len);
  block_->rpos_ -= len;
}

int32 InputBuf::ReadFd(int fd, uint32 total_len, int32* err_no) {
  block_->ensureLeft(total_len);
  int left = total_len;

  while (left > 0) {
    int readn = ::recv(fd, block_->wpos_, left, 0);
    if (readn == 0) return 0;
    else if (readn == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EWOULDBLOCK:
          break;

        default:
          if (err_no != NULL) {
            *err_no = errno;
          }
          PLOG(WARNING)<<"recv error";
          return -1;
        }  // end switch.
      }

    DCHECK_GT(readn, 0);
    left -= readn;
    block_->wpos_ += readn;
    DCHECK_LE(block_->wpos_, block_->end_);
  }

  return total_len - left;
}

}
