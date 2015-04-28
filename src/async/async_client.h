#pragma once

#include "base/base.h"

namespace io {
class OutputObject;
}

namespace async {
class Protocol;
class Connection;
class EventManager;

class AsyncClient {
  public:
    virtual ~AsyncClient();

    // not thread safe.
    void setProtocol(Protocol* p) {
      protocol_ = p;
    }

    // not thread safe.
    // be called when connection abort.
    void setCloseClosure(Closure* c) {
      close_closure_.reset(c);
    }

    // not thread safe.
    // be called when connection reconnect successfully.
    void setReconnectClosure(Closure* c) {
      reconnect_closure_.reset(c);
    }

    // thread safe.
    // please set protocol and Closures first.
    bool connect(uint32 time_out);
    void stop();

    void send(io::OutputObject* obj);

  protected:
    // ev_mgr must be initialized successfully.
    explicit AsyncClient(EventManager* ev_mgr)
        : ev_mgr_(ev_mgr), protocol_(nullptr) {
      DCHECK_NOTNULL(ev_mgr);
      timeout_ = 0;
    }

    uint32 timeout_;
    EventManager* ev_mgr_;
    scoped_ref<Connection> conn_;

    Protocol* protocol_;
    scoped_ptr<Closure> close_closure_;
    scoped_ptr<Closure> reconnect_closure_;

    void handleConnectionAbort();
    void sendInternal(io::OutputObject* obj);
    void stopInternal(SyncEvent* ev = nullptr);

    virtual bool doConnect(int* fd) = 0;
    void connectInternal(bool* success, SyncEvent* ev = nullptr);

  private:
    DISALLOW_COPY_AND_ASSIGN(AsyncClient);
};

class TcpAsyncClient : public AsyncClient {
  public:
    TcpAsyncClient(EventManager* ev_mgr, const std::string& ip, uint16 port)
        : AsyncClient(ev_mgr), ip_(ip), port_(port) {
      DCHECK(!ip.empty());
    }
    virtual ~TcpAsyncClient() {
    }

    const std::string& ip() const {
      return ip_;
    }
    uint16 port() const {
      return port_;
    }

  private:
    const std::string ip_;
    uint16 port_;

    virtual bool doConnect(int* fd);

    DISALLOW_COPY_AND_ASSIGN(TcpAsyncClient);
};

class LocalAsyncClient : public AsyncClient {
  public:
    LocalAsyncClient(EventManager* ev_mgr, const std::string& path)
        : AsyncClient(ev_mgr), path_(path) {
      DCHECK(!path.empty());
    }
    virtual ~LocalAsyncClient() {
    }

    const std::string& path() const {
      return path_;
    }

  private:
    const std::string path_;

    virtual bool doConnect(int* fd);

    DISALLOW_COPY_AND_ASSIGN(LocalAsyncClient);
};

}

