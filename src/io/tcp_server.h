#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "io/connection.h"
#include "include/object_saver.h"

namespace io {
class Protocol;
class Connection;

class EventPooler;
class EventManager;

class TcpServer : public ObjectMapSaver<int, Connection> {
  public:
    class Listener {
      public:
        virtual ~Listener() {
        }

        virtual bool doBind(const std::string& ip, uint16 port) = 0;
    };

    // event_manager must initialized successfully.
    TcpServer(EventManager* ev_mgr, uint8 worker);
    ~TcpServer();

    void setProtocol(Protocol* p) {
      protocol_ = p;
    }

    EventManager* getPoller();

    bool bindIp(const std::string& ip, uint16 port);
    void unBindIp(const std::string& ip);
    void unBindAll();

  private:
    uint8 worker_;
    Protocol* protocol_;

    EventManager* ev_mgr_;
    scoped_ptr<EventPooler> event_poller_;
    std::map<std::string, Listener*> listeners_;

    DISALLOW_COPY_AND_ASSIGN(TcpServer);
};
}

#endif /* TCP_SERVER_H_ */
