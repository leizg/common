#ifndef EVENT_PIPE_H_
#define EVENT_PIPE_H_

#include "base/base.h"

namespace io {

class EventPipe {
  public:
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        virtual void handlevent() = 0;
    };

    virtual ~EventPipe() {
      destory();
    }

    int readPipeFd() const {
      return event_fd_[0];
    }
    void handlePipeRead();

  protected:
    explicit EventPipe(Delegate* delegate)
        : deletate_(delegate) {
      DCHECK_NOTNULL(delegate);
      event_fd_[0] = INVALID_FD;
      event_fd_[1] = INVALID_FD;
    }

    bool initPipe();
    void destory();

    void triggerPipe();

  private:
    int event_fd_[2];
    scoped_ptr<Delegate> deletate_;

    DISALLOW_COPY_AND_ASSIGN(EventPipe);
};
}
#endif /* EVENT_PIPE_H_ */
