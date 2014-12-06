#include "log_reader.h"

namespace util {

bool LogReader::loadCache() {
  DCHECK_EQ(offset_, load_size_);
  int readn = log_file_->read(block_, BLOCK_SIZE);
  if (readn > 0) {
    offset_ = 0;
    load_size_ = readn;
    return true;
  }

  return false;
}

bool LogReader::readRecord(std::string* log, bool* is_last) {
  char* data = block_ + offset_;
  uint32 len = v32(data);
  data += 4;
  DCHECK_LE(len, load_size_ - offset_);

  uint32 type = v32(data);
  data += 4;
  if (type & START_RECORD) log->clear();
  if (type & LAST_RECORD) *is_last = true;

  uint32 saved_crc = v32(data);
  data += 4;
  if (saved_crc != 0) {
    // TODO: check crc32.
  }

  log->append(data, len);
  data += len;

  return true;
}

bool LogReader::read(std::string* log) {
  bool is_last = false;

  while (!is_last) {
    if (offset_ == load_size_) {
      if (!loadCache()) return false;
      continue;
    }

    DCHECK_NE(offset_, load_size_);
    if (!readRecord(log, &is_last)) {
      return false;
    }
  }

  return true;
}

}

