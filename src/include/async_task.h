#pragma once

#include "async/event/event_manager.h"

// self deleted.
class AsyncContext : public Closure {
  public:
    virtual ~AsyncContext() {
    }

    class UserData {
      public:
        virtual ~UserData() {
        }
    };
    void setUserData(UserData* ud) {
      ud_.reset(ud);
    }
    UserData* getUserData() const {
      return ud_.get();
    }

    void doSwitch() {
      ev_mgr_->runInLoop(this);
    }

  protected:
    explicit AsyncContext(async::EventManager* ev_mgr)
        : ev_mgr_(ev_mgr) {
      if (ev_mgr == nullptr) {
        ev_mgr_ = async::EventManager::current();
      }
    }

    scoped_ptr<UserData> ud_;
    async::EventManager* ev_mgr_;

  private:
    DISALLOW_COPY_AND_ASSIGN(AsyncContext);
};

// self deleted.
class AsyncTask : public Closure {
  public:
    virtual ~AsyncTask() {
    }

    void setDoneClosure(Closure* cb) {
      done_.reset(cb);
    }

  protected:
    explicit AsyncTask(AsyncContext* ctx)
        : ctx_(ctx) {
      DCHECK_NOTNULL(ctx);
    }

    scoped_ptr<AsyncContext> ctx_;
    virtual void runInternal() = 0;

  private:
    scoped_ptr<Closure> done_;
    virtual void Run() {
      runInternal();
      if (done_.get() != nullptr) {
        done_->Run();
      }
      auto cb = ctx_.release();
      cb->doSwitch();
      delete this;
    }

    DISALLOW_COPY_AND_ASSIGN(AsyncTask);
};
