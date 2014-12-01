#include <glog/logging.h>
#include <google/gflags.h>

#include "echo_client.h"
#include "io/event_manager.h"

DEFINE_string(ip, "0.0.0.0", "ip for echo server");
DEFINE_int32(port, 8888, "port for echo server");

DEFINE_int32(count, 1, "how many times per client");
DEFINE_int32(client_number, 1, "number of clients used for testing");

int main(int argc, char* argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::google::ParseCommandLineFlags(&argc, &argv, true);

  scoped_ptr<io::EventManager> ev_mgr;
  ev_mgr.reset(io::CreateEventManager());
  if (ev_mgr == NULL || !ev_mgr->Init()) {
    LOG(WARNING)<< "create event manager error";
    return -1;
  }
  ev_mgr->LoopInAnotherThread();

  bool start_ok = true;
  std::vector<test::EchoClient*> clients;
  for (uint32 i = 0; i < FLAGS_client_number; ++i) {
    test::EchoClient* cli = new test::EchoClient(ev_mgr.get(), FLAGS_count);
    if (!cli->connect(FLAGS_ip, FLAGS_port)) {
      LOG(WARNING)<< "connect error";
      start_ok = false;
    }
    clients.push_back(cli);
  }

  if (start_ok) {
    for (uint32 i = 0; i < clients.size(); ++i) {
      test::EchoClient* cli = clients[i];
      cli->startTest();
    }

    for (uint32 i = 0; i < clients.size(); ++i) {
      test::EchoClient* cli = clients[i];
      cli->waitForFinished();
    }
  }

  STLClear(&clients);

  return 0;
}
