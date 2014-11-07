#ifndef TIME_STAMP_H_
#define TIME_STAMP_H_

#include <stddef.h>
#include <sys/time.h>

class TimeStamp {
  public:
    explicit TimeStamp(const timeval& tv)
        : stamp_(tv) {
    }
    explicit TimeStamp(uint64 micro_secs) {
      stamp_.tv_sec = micro_secs / 1000;
      stamp_.tv_usec = micro_secs % 1000;
    }

    static TimeStamp afterSeconds(uint64 secs);
    static TimeStamp afterMicroSeconds(uint64 micro_secs);

    uint64 microSecs() const {
      return stamp_.tv_sec * 1000 + stamp_.tv_usec;
    }

    const timeval& timeVal() const {
      return stamp_;
    }
    timespec timeSpec() const {
      timespec ts = { 0 };
      ts.tv_sec = stamp_.tv_sec;
      ts.tv_nsec = stamp_.tv_usec * 1000;
      return ts;
    }

    bool operator <(const TimeStamp& t) const {
      if (stamp_.tv_sec < t.stamp_.tv_sec) return true;
      return stamp_.tv_usec < t.stamp_.tv_usec;
    }
    bool operator >(const TimeStamp& t) const {
      return !operator <(t);
    }

  private:
    timeval stamp_;
};

inline TimeStamp Now() {
  timeval tv;
  ::gettimeofday(&tv, NULL);
  return TimeStamp(tv);
}

inline TimeStamp TimeStamp::afterMicroSeconds(uint64 micro_secs) {
  return TimeStamp(Now().microSecs() + micro_secs);
}
inline TimeStamp TimeStamp::afterSeconds(uint64 secs) {
  return TimeStamp(Now().microSecs() + secs * 1000);
}

inline TimeStamp TimeAdd(const TimeStamp& t, uint sec) {
  return TimeStamp(t.microSecs() + sec * 1000);
}
inline uint64 TimeDiff(const TimeStamp& t1, const TimeStamp& t2) {
  return t1.microSecs() - t2.microSecs();
}

#endif /* TIME_STAMP_H_ */
