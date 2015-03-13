#include "input_stream.h"

namespace io {

bool InputStream::read(const char** buf, uint64* len) const {
  for (; index_ < iov_.size(); ++index_) {
    const iovec& io = iov_[index_];
    DCHECK_GE(io.iov_len, offset_);

    if (offset_ != io.iov_len) {
      *len = std::min(*len, io.iov_len - offset_);
      *buf = static_cast<char*>(io.iov_base) + offset_;
      offset_ += *len;
      total_ += *len;
      return true;
    }

    offset_ = 0;
  }

  return false;
}

void InputStream::skip(uint64 len) const {
  uint64 left = len;
  for (; left > 0 && index_ < iov_.size(); ++index_) {
    DCHECK_LT(index_, iov_.size());
    const iovec& io = iov_[index_];
    DCHECK_GE(io.iov_len, offset_);

    uint32 avail = io.iov_len - offset_;
    if (avail != 0) {
      if (avail >= left) {
        offset_ += avail;
        left = 0;
        break;
      }
      left -= avail;
    }
    offset_ = 0;
  }

  total_ += len - left;
}

void InputStream::backup(uint64 len) const {
  uint64 left = len;
  while (left > 0) {
    const iovec& io = iov_[index_];
    DCHECK_GE(io.iov_len, offset_);

    if (offset_ >= left) {
      offset_ -= len;
      left = 0;
      break;
    } else if (offset_ < left) {
      left -= offset_;
      offset_ = 0;
    }

    if (index_ == 0) break;
    offset_ = iov_[--index_].iov_len;
  }

  total_ -= len - left;
}

}
