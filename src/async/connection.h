#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "include/object_saver.h"

namespace io {
class OutQueue;
class InputStream;
class OutputObject;
}

namespace net {
class Protocol;

struct Event;
class EventManager;

// Note: Connection shouldn't delete directly.
class Connection : public RefCounted {
  public:
    struct Attr {
        virtual ~Attr() {
        }
        virtual void Init() = 0;

        uint32 io_stat;
        uint32 pending_size;
        uint32 data_len;

        bool is_last_pkg;
    };

    Connection(int fd, EventManager* ev_mgr);
    virtual ~Connection();

    void Init();

    const int& FileHandle() const {
      return fd_;
    }
    EventManager* getEventLoop() const {
      return ev_mgr_;
    }

    void setSaver(ObjectSaver<int, Connection>* saver) {
      DCHECK_NE(saver_, saver);
      if (saver_ != NULL) {
        saver_->Remove(fd_);
        saver_ = NULL;
      }
      if (saver != NULL) {
        saver->Add(fd_, this);
        saver_ = saver;
      }
    }

    void setAttr(Attr* attr);
    Attr* getAttr() const {
      return attr_.get();
    }
    void setProtocol(Protocol* p) {
      protocol_ = p;
    }

    void setCloseClosure(Closure* cb, bool run_old_if_exist = false) {
      if (run_old_if_exist && close_closure_.get() != NULL) {
        close_closure_->Run();
      }
      close_closure_.reset(cb);
    }

    // out_obj will deleted by connection.
    // Note: not thread safe.
    void Send(io::OutputObject* out_obj);
    int32 Recv(uint32 len, int* err_no);

    void handleRead(const TimeStamp& time_stamp);
    void handleWrite(const TimeStamp& time_stamp);

    void ShutDown();

  private:
    int fd_;
    bool closed_;
    EventManager* ev_mgr_;
    scoped_ptr<Event> event_;

    Protocol* protocol_;
    scoped_ptr<Attr> attr_;

    ObjectSaver<int, Connection>* saver_;

    scoped_ptr<Closure> close_closure_;

    scoped_ptr<io::InputStream> input_stream_;
    scoped_ptr<io::OutQueue> out_queue_;

    DISALLOW_COPY_AND_ASSIGN(Connection);
};

}

#endif /* CONNECTION_H_ */
