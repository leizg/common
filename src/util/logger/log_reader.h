#ifndef LOG_READER_H_
#define LOG_READER_H_

#include "base/base.h"
#include "logger_def.h"

namespace util {

class LogReader {
  public:
    LogReader(SequentialReadonlyFile* log_file, bool enable_crc_check = true)
        : log_file_(log_file), crc_check_(enable_crc_check), offset_(0), load_size_(
            0) {
      DCHECK_NOTNULL(log_file);
    }
    ~LogReader() {
    }

    bool read(std::string* log);
    bool read(std::deque<std::string*>* log_vec, int size) {
      for (uint32 i = 0; i < size; ++i) {
        std::string* log = new std::string;
        if (!read(log)) {
          delete log;
          break;
        }
        log_vec->push_back(log);
      }
      return !log_vec->empty();
    }

  private:
    scoped_ptr<SequentialReadonlyFile> log_file_;

    const bool crc_check_;

    uint32 offset_;
    uint32 load_size_;
    char block_[BLOCK_SIZE];

    bool loadCache();
    bool readRecord(std::string* log, bool* is_last);

    DISALLOW_COPY_AND_ASSIGN(LogReader);
};
}
#endif /* LOG_READER_H_ */
