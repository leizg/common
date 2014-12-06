#include "util/logger/log_reader.h"
#include "util/logger/log_writer.h"

#include <gtest/gtest.h>

DEFINE_string(logger_file, "/tmp/log", "file used for testing log.");

namespace test {

class LoggerTest : public testing::Test {
  public:
    LoggerTest()
        : log_file_(FLAGS_logger_file) {
      DCHECK(!FLAGS_logger_file.empty());
      test_log_ = std::string("womendoushihaowawa!");
    }
    virtual ~LoggerTest() {
    }

  protected:
    scoped_ptr<util::LogReader> reader_;
    scoped_ptr<util::LogWriter> writer_;

    bool InitWriter();
    bool InitReader();

    std::string test_log_;

  private:
    const std::string log_file_;

    DISALLOW_COPY_AND_ASSIGN(LoggerTest);
};

bool LoggerTest::InitWriter() {
  AppendonlyMmapedFile* f = new AppendonlyMmapedFile(log_file_);
  if (!f->Init()) {
    delete f;
    LOG(WARNING)<< "init log file error, path: " << log_file_;
    return false;
  }

  writer_.reset(new util::LogWriter(f));
  return true;
}

bool LoggerTest::InitReader() {
  SequentialReadonlyFile* f = new SequentialReadonlyFile(log_file_);
  if (!f->Init()) {
    delete f;
    LOG(WARNING)<< "init log file error, path: " << log_file_;
    return false;
  }

  reader_.reset(new util::LogReader(f));
  return true;
}

TEST_F(LoggerTest, writeLog) {
  ASSERT_TRUE(InitWriter());
  ASSERT_TRUE(writer_->append(test_log_));
  writer_->flush();
  writer_.reset();
}

TEST_F(LoggerTest, readLog) {
  ASSERT_TRUE(InitReader());
  std::string log;
  ASSERT_TRUE(reader_->read(&log));
  reader_.reset();

  ASSERT_EQ(log, test_log_);
}

}
