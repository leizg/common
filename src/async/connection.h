#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "include/object_saver.h"

namespace async {
struct Event;
class Protocol;
class EventManager;

// Note: Connection shouldn't delete directly.
class Connection : public RefCounted {
  public:
    struct UserData {
        virtual ~UserData() {
        }

        Connection* conn;
    };

    Connection(int fd, EventManager* ev_mgr)
        : fd_(fd), closed_(false), ev_mgr_(ev_mgr) {
      DCHECK_NE(fd, INVALID_FD);
      DCHECK_NOTNULL(ev_mgr);
      saver_ = nullptr;
      protocol_ = nullptr;
    }
    virtual ~Connection();

    bool init();

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

    void setSaver(ObjectSaver<int, Connection>* saver) {
      DCHECK_NE(saver_, saver);
      if (saver_ != nullptr) {
        saver_->Remove(fd_);
        saver_ = nullptr;
      }
      if (saver != nullptr) {
        saver->Add(fd_, this);
        saver_ = saver;
      }
    }

    void setData(UserData* attr);
    UserData* getData() const {
      return data_.get();
    }

    bool read(char* buf, int32* len);
    bool write(const char* buf, int32* len);
    bool write(int fd, off_t offset, int32* len);
    bool write(const std::vector<iovec>& iov, int32* len);

    void handleRead(TimeStamp time_stamp);
    void handleWrite(TimeStamp time_stamp);

    void updateChannel(uint8 event);
    void shutDownFromServer();

  private:
    int fd_;
    bool closed_;

    EventManager* ev_mgr_;
    scoped_ptr<Event> event_;

    Protocol* protocol_;
    scoped_ptr<Closure> close_closure_;
    scoped_ptr<UserData> data_;

    ObjectSaver<int, Connection>* saver_;

    DISALLOW_COPY_AND_ASSIGN(Connection);
};

}

#endif /* CONNECTION_H_ */
