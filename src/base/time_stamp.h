#ifndef TIME_STAMP_H_
#define TIME_STAMP_H_

#include <stddef.h>
#include <sys/time.h>

class TimeStamp {
 public:
  TimeStamp() {
    timeval tv;
    ::gettimeofday(&tv, NULL);
    stamp_ = tv.tv_sec * 1000 + tv.tv_usec;
  }
  explicit TimeStamp(time_t tm)
      : stamp_(tm) {
  }

  time_t MicroSecs() const {
    return stamp_;
  }
  timespec TimeSpec() const {
    timespec ts = { 0 };
    ts.tv_sec = stamp_ / 1000;
    ts.tv_nsec = (stamp_ % 1000) * 1000;
    return ts;
  }

  bool operator <(time_t t) const {
    return stamp_ < t;
  }
  bool operator >(time_t t) const {
    return !operator <(t);
  }

 private:
  time_t stamp_;
};

bool operator <(const TimeStamp& t1, const TimeStamp& t2) {
  return t1 < t2.MicroSecs();
}
bool operator >(const TimeStamp& t1, const TimeStamp& t2) {
  return !operator <(t1, t2);
}

inline TimeStamp Now() {
  return TimeStamp();
}
inline TimeStamp TimeAdd(const TimeStamp& t, uint sec) {
  return TimeStamp(t.MicroSecs() + sec * 1000);
}
inline uint64 TimeDiff(const TimeStamp& t1, const TimeStamp& t2) {
  return t1.MicroSecs() - t2.MicroSecs();
}

#endif /* TIME_STAMP_H_ */
