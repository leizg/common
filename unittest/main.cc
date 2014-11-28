#include <gtest/gtest.h>

#include <glog/logging.h>
#include <google/gflags.h>

int main(int argc, char* argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  ::google::ParseCommandLineFlags(&argc, &argv, true);

  return RUN_ALL_TESTS();
}
