#include "input_buf.h"

namespace io {

int32 InputBuf::Next(char** buf, uint32* len) {
  if (wpos_ == rpos_) return 0;

  uint32 alival = std::min((uint32) (wpos_ - rpos_), *len);
  *len = alival;
  *buf = rpos_;

  rpos_ += alival;
  CHECK_LE(rpos_, wpos_);

  return *len;
}

void InputBuf::EnsureLeft(uint32 len) {
  if (left() < len) {
    uint32 rn = readn(), wn = writen();
    uint32 new_size = capacity() * 2;
    uint32 left_size = new_size - wn;
    if (left_size < len) {
      new_size += len - left_size;
    }

    mem_ = ::realloc(mem_, new_size);
    end_ = mem_ + new_size;
    rpos_ = mem_ + rn;
    wpos_ = mem_ + wn;
  }
}

int32 InputBuf::ReadFd(int fd, uint32 total_len, int32* err_no) {
  EnsureLeft(total_len);

  uint32 left_len = total_len;
  while (left_len > 0) {
    int n = ::recv(fd, wpos_, left_len, 0);
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
    wpos_ += n;
  }

  return total_len - left_len;
}

}
