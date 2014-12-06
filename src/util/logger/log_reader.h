#ifndef LOG_READER_H_
#define LOG_READER_H_

#include "base/base.h"
#include "logger_def.h"

namespace util {

class LogReader {
  public:
    explicit LogReader(SequentialReadonlyFile* log_file)
        : log_file_(log_file), offset_(0), load_size_(0) {
      DCHECK_NOTNULL(log_file);
    }
    ~LogReader() {
    }

    bool read(std::string* log);

  private:
    scoped_ptr<SequentialReadonlyFile> log_file_;

    uint32 offset_;
    uint32 load_size_;
    char block_[BLOCK_SIZE];

    bool loadCache();
    bool readRecord(std::string* log, bool* is_last);

    DISALLOW_COPY_AND_ASSIGN(LogReader);
};
}
#endif /* LOG_READER_H_ */
