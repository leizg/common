#include "log_input_stream.h"
#include "log_reader.h"

namespace {
std::string itostring(uint64 i) {
  std::string buf;
  while (i != 0) {
    char c = i % 10 + '0';
    buf.append(0, c);
    i /= 10;
  }
  return buf;
}
}

namespace util {

class LogInputStream::LogDir {
  public:
    explicit LogDir(const std::string& dir)
        : dit_(dir), dir_name_(dir) {
    }
    ~LogDir() {
    }

    bool Init();

    bool nextLogFile(std::string* log_file);

  private:
    DirIterator dit_;
    const std::string dir_name_;

    std::deque<std::string> log_files_;

    DISALLOW_COPY_AND_ASSIGN(LogDir);
};

bool LogInputStream::LogDir::Init() {
  if (!dit_.Init()) return false;

  std::set<uint64> ids;
  while (true) {
    const std::string* file = dit_.next(DirIterator::REG_FILE);
    if (file != NULL) break;

    DCHECK(!file->empty());
    if (file->at(0) != '.') {
      ids.insert(atoi(file->c_str()));
    }
  }

  for (auto i = ids.begin(); i != ids.end(); ++i) {
    log_files_.push_back(itostring(*i));
  }
  return true;
}

bool LogInputStream::LogDir::nextLogFile(std::string* log_file) {
  if (!log_files_.empty()) {
    *log_file = dir_name_ + "/" + log_files_.front();
    log_files_.pop_front();
    return true;
  }

  return false;
}

LogInputStream::~LogInputStream() {
  STLClear(&log_queue_);
}

bool LogInputStream::Init(const std::string& log_dir) {
  dir_.reset(new LogDir(log_dir));
  if (!dir_->Init()) return false;

  loadCache();
  return true;
}

bool LogInputStream::createReader() {
  std::string log_file;

  SequentialReadonlyFile* file;
  while (dir_->nextLogFile(&log_file)) {
    file = new SequentialReadonlyFile(log_file);
    if (!file->Init()) {
      delete file;
      continue;
    }

    reader_.reset(new LogReader(file, crc32_check_));
    return true;
  }

  return false;
}

bool LogInputStream::loadCache() {
  DCHECK(log_queue_.empty());
  while (true) {
    if (reader_ == NULL && !createReader()) {
      return false;
    }

    DCHECK_NOTNULL(reader_.get());
    if (!reader_->read(&log_queue_, kLoadNumber)) {
      reader_.reset();
      continue;
    }
    break;
  }

  DCHECK(!log_queue_.empty());
  return true;
}

std::string* LogInputStream::next() {
  if (log_queue_.empty()) {
    if (!loadCache()) return NULL;
    if (log_queue_.empty()) return NULL;
  }

  DCHECK(!log_queue_.empty());
  std::string* log = log_queue_.front();
  log_queue_.pop_front();
  return log;
}

}
