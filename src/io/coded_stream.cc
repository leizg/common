#include "coded_stream.h"
#include "input_stream.h"
#include "output_stream.h"

namespace io {

CodedInputStream::~CodedInputStream() {
  if (auto_release_) delete stream_;
}

bool CodedInputStream::readBytes(char* buf, uint64 len) const {
  const char* data;
  uint64 offset = 0;
  for (uint64 size = len; size != 0; size = len - offset) {
    if (!stream_->Next(&data, &size)) {
      return false;
    }
    ::memcpy(buf + offset, data, size);
    offset += size;
  }

  return true;
}

void CodedOutputStream::writeBytes(const char* data, uint64 len) {
  char* ptr;
  uint64 offset = 0;
  for (uint64 size = len; size != 0; size = len - offset) {
    if (!stream_->Next(&ptr, &size)) {
      return;
    }

    ::memcpy(ptr, data + offset, size);
    offset += size;
  }
}

}

