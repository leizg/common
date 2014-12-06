#include "log_writer.h"

namespace util {

bool LogWriter::append(uint32 type, const char* data, uint32 len) {
  uint32 space_len = BLOCK_SIZE - block_offset_;
  if (space_len < LOG_HEADER_SIZE + len) {
    return false;
  }

  // length + type + crc32
  char* buf = block_ + block_offset_;
  save32(buf, len);  // length
  buf += 4;
  save32(buf, type);  // type
  buf += 4;
  save32(buf, 0);  // crc32, TODO
  buf += 4;

  ::memcpy(buf, data, len);
  block_offset_ += LOG_HEADER_SIZE + len;
  return true;
}

void LogWriter::flush() {
  if (block_offset_ != 0) {
    int32 writen = log_file_->write(block_, block_offset_);
    DCHECK_EQ(writen, block_offset_);

    block_offset_ = 0;
  }
}

bool LogWriter::append(const char* data, uint32 len) {
  for (uint32 pos = 0; pos != len; /* empty */) {
    uint32 space_size = BLOCK_SIZE - block_offset_;
    if (space_size <= LOG_HEADER_SIZE) {
      ::memset(block_ + block_offset_, '\0', space_size);
      flush();
      continue;
    }

    DCHECK_GT(space_size, LOG_HEADER_SIZE);
    uint32 avail_len = std::min(space_size, len - pos);
    uint32 type = 0;
    if (pos == 0) type |= START_RECORD;
    if (pos + avail_len == len) type |= LAST_RECORD;

    if (!append(type, data + pos, avail_len)) {
      return false;
    }

    pos += avail_len;
  }

  return true;
}

}

