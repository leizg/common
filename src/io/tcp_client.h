#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "base/base.h"

namespace io {
class EventManager;

class TcpClient {
 public:
  class Connector {
   public:
    virtual ~Connector() {
    }

    // return -1 iff connect fail or timedout.
    virtual int Connect(const std::string& ip, uint16 port,
                        uint32 timeout) const = 0;
  };

  TcpClient(EventManager* ev, const std::string& ip, uint16 port);
  ~TcpClient();

  void SetCloseClosure(Closure* c) {
    close_slosure_.reset(c);
  }

  bool Connect(uint32 time_out);

 private:
  EventManager* ev_;
  const std::string ip_;
  uint16 port_;

  scoped_ptr<Connector> connector_;
  scoped_ref<Connection> connection_;

  scoped_ptr<Closure> close_slosure_;

  DISALLOW_COPY_AND_ASSIGN(TcpClient);
};
}

#endif /* TCP_CLIENT_H_ */
