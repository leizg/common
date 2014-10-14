#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "base/base.h"

namespace io {
class Protocol;
class EventManager;
class OutputObject;

// Note: Connection shouldn't delete directly.
class Connection : public RefCounted {
 public:
  struct Attr {
    virtual void Init() = 0;

    bool is_last_pkg;
    uint32 io_stat;
    uint32 pending_size;
  };

  Connection(int fd, EventManager* ev_mgr);
  bool Init();

  int FileHandle() const {
    return fd_;
  }
  EventManager* getEventLoop() const {
    return ev_mgr_;
  }

  void setAttr(Attr* attr) {
    attr_.reset(attr);
  }
  Attr* getAttr() const {
    return attr_.get();
  }
  void setProtocol(Protocol* p) {
    protocol_ = p;
  }

  void setCloseClosure(Closure* cb) {
    if (close_closure_.get() != NULL) {
      close_closure_->Run();
    }
    close_closure_.reset(cb);
  }

  // out_obj will deleted by connection.
  // Note: not thread safe.
  void Send(OutputObject* out_obj);

  void handleRead();
  void handleWrite();

  void ShutDown();

 private:
  int fd_;

  EventManager* ev_mgr_;
  Protocol* protocol_;

  class OutQueue;
  scoped_ptr<OutQueue> out_queue_;
  scoped_ptr<Attr> attr_;
  scoped_ptr<Closure> close_closure_;

  virtual ~Connection();

  DISALLOW_COPY_AND_ASSIGN(Connection);

};
}

#endif /* CONNECTION_H_ */
