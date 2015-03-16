#ifndef TCP_CLIENT_IMPL_H_
#define TCP_CLIENT_IMPL_H_

#include "tcp_client.h"

namespace io {
class OutQueue;
}

namespace async {
class Connection;

class TcpClientImpl : public TcpClient {
  public:
    TcpClientImpl(EventManager* ev_mgr, const std::string& ip, uint16 port);
    virtual ~TcpClientImpl();

  private:
    virtual bool connect(uint32 time_out);
    virtual void send(io::OutputObject* out_obj);

    scoped_ref<Connection> conn_;

    void handleConnectionAbort();

    void sendInternal(io::OutputObject* out_obj);
    void connectInternal(uint32 time_out, bool* success, SyncEvent* ev = NULL);

    DISALLOW_COPY_AND_ASSIGN(TcpClientImpl);
};

}

#endif /* TCP_CLIENT_IMPL_H_ */
