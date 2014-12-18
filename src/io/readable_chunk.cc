#include "readable_chunk.h"

namespace io {

int32 ReadFdChunk::ReadFd(int fd, uint32 total_len, int32* err_no) {
  wchunk_->ensureLeft(total_len);
  int left = total_len;

  char* data = wchunk_->peekW();
  while (left > 0) {
    int readn = ::recv(fd, data, left, 0);
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
    data += readn;
    wchunk_->skip(readn);
  }

  return total_len - left;
}

}
