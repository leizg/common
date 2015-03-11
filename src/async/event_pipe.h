#ifndef EVENT_PIPE_H_
#define EVENT_PIPE_H_

#include "base/base.h"

namespace async {
struct Event;
class EventManager;

class EventPipe {
  public:
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        virtual void handleEvent(TimeStamp ts) = 0;
    };

    virtual ~EventPipe() {
      destory();
    }

    virtual bool init();

    void handleRead(TimeStamp ts);

  protected:
    EventPipe(EventManager* ev_mgr, Delegate* delegate)
        : ev_mgr_(ev_mgr), deletate_(delegate) {
      DCHECK_NOTNULL(ev_mgr);
      DCHECK_NOTNULL(delegate);
      event_fd_[0] = INVALID_FD;
      event_fd_[1] = INVALID_FD;
    }

    EventManager* ev_mgr_;

    void destory();
    void triggerPipe();

  private:
    int event_fd_[2];
    scoped_ptr<Event> event_;

    scoped_ptr<Delegate> deletate_;

    DISALLOW_COPY_AND_ASSIGN(EventPipe);
};
}
#endif /* EVENT_PIPE_H_ */
