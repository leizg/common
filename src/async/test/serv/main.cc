#include <glog/logging.h>
#include <google/gflags.h>

#include "echo_server.h"

DEFINE_string(ip, "0.0.0.0", "ip for echo server");
DEFINE_int32(port, 8888, "port for echo server");
DEFINE_int32(worker, 4, "number of worker thread");

int main(int argc, char* argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::google::ParseCommandLineFlags(&argc, &argv, true);

  scoped_ptr<test::EchoServer> serv(new test::EchoServer(FLAGS_worker));
  if (!serv->Init(FLAGS_ip, FLAGS_port)) {
    return -1;
  }

  serv->loop(true);

  const std::string stop_file("/tmp/.serv.stop");
  while (!FileExist(stop_file)) {
    sleep(2);
  }

  serv.reset();
  return 0;
}
