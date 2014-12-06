#ifndef LOG_WRITER_H_
#define LOG_WRITER_H_

#include "base/base.h"
#include "logger_def.h"

namespace util {

// not threadsafe.
class LogWriter {
  public:
    LogWriter(AppendonlyMmapedFile* log_file, uint64 limit_size,
              COPMRESS_TYPE compress_type)
        : log_file_(log_file), block_offset_(0) {
    }
    ~LogWriter() {
    }

    void flush();
    bool append(const char* data, uint32 len);

  private:
    scoped_ptr<AppendonlyMmapedFile> log_file_;

    uint32 block_offset_;
    char block_[BLOCK_SIZE];

    bool append(uint32 type, const char* data, uint32 len);

    DISALLOW_COPY_AND_ASSIGN(LogWriter);
};
}
#endif /* LOG_WRITER_H_ */
