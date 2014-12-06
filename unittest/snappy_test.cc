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
    scoped_ptr<util::StringInBuf> in_buf_;
    scoped_ptr<util::StringOutBuf> out_buf_;

    void SetUp() {
      origin_data_ = "women dou shi hao wawa!";
      cp_.reset(new util::SnappyCompression);

      in_buf_.reset(new util::StringInBuf(&origin_data_));
      out_buf_.reset(new util::StringOutBuf);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(SnappyTest);
};

TEST_F(SnappyTest, SimpleTest) {
  ASSERT_TRUE(cp_->compress(in_buf_.get(), out_buf_.get()));

  util::StringInBuf in2(&out_buf_->data());
  util::StringOutBuf out2;
  ASSERT_TRUE(cp_->decompress(&in2, &out2));

  ASSERT_EQ(origin_data_, out2.data());
}
}

