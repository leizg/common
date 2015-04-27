#pragma once

#include "base/base.h"

namespace async {
struct Event;
class Protocol;
class EventManager;

// Note: Connection shouldn't delete directly.
class Connection : public RefCounted {
  public:
    Connection(int fd, EventManager* ev_mgr);
    virtual ~Connection();

    bool init();
    void shutDown();

    int fileHandle() const {
      return fd_;
    }
    EventManager* getEventLoop() const {
      return ev_mgr_;
    }

    void setProtocol(Protocol* p) {
      protocol_ = p;
    }
    void setCloseClosure(Closure* cb, bool run_old_if_exist = false) {
      if (run_old_if_exist && close_closure_ != nullptr) {
        close_closure_->Run();
      }
      close_closure_.reset(cb);
    }

    bool read(char* buf, int32* len);
    bool write(const char* buf, int32* len, int* err_no);
    bool write(int fd, off_t offset, int32* len, int* err_no);
    bool write(const std::vector<iovec>& iov, int32* len, int* err_no);

    void handleClose();
    void handleRead(TimeStamp time_stamp);
    void handleWrite(TimeStamp time_stamp);

    struct UserData {
        virtual ~UserData() {
        }

        Connection* conn;
    };
    void setData(UserData* data) {
      data_.reset(data);
    }
    UserData* getData() const {
      return data_.get();
    }

  private:
    int fd_;
    bool closed_;

    EventManager* ev_mgr_;
    scoped_ptr<Event> event_;

    Protocol* protocol_;
    scoped_ptr<Closure> close_closure_;
    scoped_ptr<UserData> data_;

    void updateChannel(uint8 event);
    void shutDownInternal(SyncEvent* ev = nullptr);

    DISALLOW_COPY_AND_ASSIGN(Connection);
};
}
