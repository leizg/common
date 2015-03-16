#ifndef ECHO_CLIENT_H_
#define ECHO_CLIENT_H_

#include "base/base.h"

namespace async {
class Protocol;
class TcpClient;
class EventManager;
}

namespace test {

class EchoClient {
  public:
    EchoClient(async::EventManager* ev_mgr, uint32 count);
    ~EchoClient();

    bool connect(const std::string& ip, uint16 port);

    void startTest();
    void waitForFinished();

  private:
    uint32 test_number_;

    async::EventManager* ev_mgr_;
    scoped_ptr<async::Protocol> protocol_;
    scoped_ptr<async::TcpClient> client_;

    scoped_ptr<StoppableThread> thread_;

    DISALLOW_COPY_AND_ASSIGN(EchoClient);
};
}
#endif /* ECHO_CLIENT_H_ */
