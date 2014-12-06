#include "util/compression/snappy.h"
#include <gtest/gtest.h>

namespace test {
class SnappyTest : public testing::Test {
  public:
    SnappyTest() {
    }
    virtual ~SnappyTest() {
    }

  protected:
    scoped_ptr<util::SnappyCompression> cp_;
    std::string origin_data_;

    void SetUp() {
      origin_data_ = "women dou shi hao wawa!";
      cp_.reset(new util::SnappyCompression);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(SnappyTest);
};

TEST_F(SnappyTest, SimpleTest) {
  util::StringInBuf in1(&origin_data_);
  util::StringOutBuf out1;
  ASSERT_TRUE(cp_->compress(&in1, &out1));

  util::StringInBuf in2(&out1.data());
  util::StringOutBuf out2;
  ASSERT_TRUE(cp_->decompress(&in2, &out2));

  ASSERT_EQ(origin_data_, out2.data());
}
}

