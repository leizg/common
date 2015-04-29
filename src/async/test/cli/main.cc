#include <glog/logging.h>
#include <google/gflags.h>

#include "echo_client.h"
#include "async/event/event_manager.h"

DEFINE_string(ip, "0.0.0.0", "ip for echo server");
DEFINE_int32(port, 8888, "port for echo server");

DEFINE_int32(count, 1, "how many times per client");
DEFINE_int32(client_number, 1, "number of clients used for testing");

bool buildClients(async::EventManager* ev_mgr,
                  std::vector<test::EchoClient*>* clients) {
  for (int32 i = 0; i < FLAGS_client_number; ++i) {
    test::EchoClient* cli = new test::EchoClient(ev_mgr, FLAGS_count);
    if (!cli->connect(FLAGS_ip, FLAGS_port)) {
      LOG(WARNING)<< "connect error";
      return false;
    }

    clients->push_back(cli);
  }

  return true;
}

static void startTest(async::EventManager* ev_mgr) {
  std::vector<test::EchoClient*> clients;
  if (!buildClients(ev_mgr, &clients)) {
    STLClear(&clients);
    return;
  }

  for (auto cli : clients) {
    cli->startTest();
  }

  for (auto cli : clients) {
    cli->waitForFinished();
  }

  STLClear(&clients);
}

static async::EventManager* createEventManager() {
  scoped_ptr<async::EventManager> ev_mgr;

  ev_mgr.reset(async::CreateEventManager());
  if (ev_mgr == nullptr || !ev_mgr->init()) {
    LOG(WARNING)<< "create event manager error";
    return nullptr;
  }

  ev_mgr->loopInAnotherThread();

  return ev_mgr.release();
}

int main(int argc, char* argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::google::ParseCommandLineFlags(&argc, &argv, true);

  scoped_ptr<async::EventManager> ev_mgr(createEventManager());
  if (ev_mgr == nullptr) {
    LOG(WARNING)<< "create event manager error";
    return -1;
  }

  startTest(ev_mgr.get());

  SyncEvent ev;
  ev_mgr->stop(&ev);
  ev.Wait();

  return 0;
}
