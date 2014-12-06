#ifndef LOG_WRITER_H_
#define LOG_WRITER_H_

#include "base/base.h"
#include "logger_def.h"

namespace util {

// not threadsafe.
class LogWriter {
  public:
    LogWriter(AppendonlyMmapedFile* log_file, bool enable_crc_check = true)
        : log_file_(log_file), crc_check_(enable_crc_check), block_offset_(0) {
    }
    ~LogWriter() {
    }

    void flush();
    bool append(const char* data, uint32 len);
    bool append(const std::string& log) {
      return append(log.data(), log.size());
    }

  private:
    scoped_ptr<AppendonlyMmapedFile> log_file_;

    const bool crc_check_;

    uint32 block_offset_;
    char block_[BLOCK_SIZE];

    bool append(uint32 type, const char* data, uint32 len);

    DISALLOW_COPY_AND_ASSIGN(LogWriter);
};
}
#endif /* LOG_WRITER_H_ */
