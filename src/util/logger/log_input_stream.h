#ifndef LOG_INPUT_STREAM_H_
#define LOG_INPUT_STREAM_H_

#include "base/base.h"

namespace util {
class LogReader;

class LogInputStream {
  public:
    explicit LogInputStream(bool enable_crc32_check)
        : crc32_check_(enable_crc32_check) {
    }
    virtual ~LogInputStream();

    bool Init(const std::string& log_dir);

    std::string* next();

  private:
    bool crc32_check_;

    bool loadCache();

    std::deque<std::string*> log_queue_;

    class LogDir;
    scoped_ptr<LogDir> dir_;
    scoped_ptr<LogReader> reader_;

    DISALLOW_COPY_AND_ASSIGN(LogInputStream);
};
}

#endif /* LOG_INPUT_STREAM_H_ */
