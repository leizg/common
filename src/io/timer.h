#ifndef TIMER_H_
#define TIMER_H_

#include "base/base.h"

namespace io {

class Timer : public Closure {
 public:
  // not held the closure.
  Timer(bool is_repeated, uint32 interval, Closure* closure)
      : id_(timer_id_++),
        is_repeated_(is_repeated_),
        interval_(interval),
        closure_(closure) {
  }
  virtual ~Timer() {
  }

  uint64 id() const {
    return id_;
  }
  bool IsRepeated() const {
    return is_repeated_;
  }
  uint32 Interval() const {
    return interval_;
  }

  void Reset() {
    stamp_ = Now() + interval_;
  }

  const TimeStamp& ExpiredTime() const {
    return stamp_;
  }
  bool IsExpires(const TimeStamp& ts) const {
    return stamp_ < ts;
  }

  // by Closure.
  void Run() {
    if (closure_ != NULL) {
      closure_->Run();
    }
  }

 private:
  const uint32 id_;
  bool is_repeated_;
  uint32 interval_;

  TimeStamp stamp_;
  Closure* closure_;

  static uint64 timer_id_;

  DISALLOW_COPY_AND_ASSIGN(Timer);
};
}

#endif /* TIMER_H_ */
