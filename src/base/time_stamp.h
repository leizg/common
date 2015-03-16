#ifndef TIME_STAMP_H_
#define TIME_STAMP_H_

#include <stddef.h>
#include <sys/time.h>

class TimeStamp {
  public:
    explicit TimeStamp(uint64 micro_secs)
        : ms_(micro_secs) {
    }

    const static uint64 kMilliSecsPerSecond = 1000ULL;
    const static uint64 kMicroSecsPerMilliSecond = 1000ULL;
    const static uint64 kMicroSecsPerSecond = kMicroSecsPerMilliSecond
        * kMilliSecsPerSecond;
    const static uint64 kNanoSecsPerMicroSecond = 1000ULL;
    const static uint64 kNanoSecsPerSecond = kMicroSecsPerSecond
        * kNanoSecsPerMicroSecond;

    static TimeStamp now();
    static TimeStamp afterSeconds(uint64 secs);
    static TimeStamp afterMicroSeconds(uint64 micro_secs);

    uint64 microSecs() const {
      return ms_;
    }
    struct timeval toTimeVal() const;
    struct timespec toTimeSpec() const;

    bool operator <(const TimeStamp& t) const {
      return ms_ < t.ms_;
    }
    bool operator >(const TimeStamp& t) const {
      return !operator <(t);
    }

    TimeStamp& operator +(uint64 micro_sec) {
      ms_ += micro_sec;
      return *this;
    }
    TimeStamp& operator +(const TimeStamp& rhs) {
      ms_ += rhs.ms_;
      return *this;
    }
    TimeStamp& operator -(const TimeStamp& rhs) {
      ms_ -= rhs.ms_;
      return *this;
    }
    TimeStamp& operator -(uint64 micro_sec) {
      ms_ -= micro_sec;
      return *this;
    }

    TimeStamp& operator +=(uint64 micro_sec) {
      ms_ += micro_sec;
      return *this;
    }
    TimeStamp& operator -=(uint64 micro_sec) {
      ms_ -= micro_sec;
      return *this;
    }
    TimeStamp& operator +=(const TimeStamp& rhs) {
      ms_ += rhs.ms_;
      return *this;
    }
    TimeStamp& operator -=(const TimeStamp& rhs) {
      ms_ -= rhs.ms_;
      return *this;
    }

  private:
    uint64 ms_;
};

inline struct timeval TimeStamp::toTimeVal() const {
  struct timeval tv = { 0 };
  tv.tv_sec = ms_ / kMicroSecsPerSecond;
  tv.tv_usec = ms_ % kMicroSecsPerSecond;
  return tv;
}

inline struct timespec TimeStamp::toTimeSpec() const {
  timespec ts = { 0 };
  ts.tv_sec = ms_ / kMicroSecsPerSecond;
  ts.tv_nsec = ms_ % kMicroSecsPerSecond * kNanoSecsPerMicroSecond;
  return ts;
}

inline TimeStamp TimeStamp::now() {
  timeval tv;
  ::gettimeofday(&tv, NULL);
  return TimeStamp(tv.tv_sec * TimeStamp::kMicroSecsPerSecond + tv.tv_usec);
}
inline TimeStamp TimeStamp::afterMicroSeconds(uint64 micro_secs) {
  return TimeStamp(now().microSecs() + micro_secs);
}
inline TimeStamp TimeStamp::afterSeconds(uint64 secs) {
  return TimeStamp(now().microSecs() + secs * 1000);
}

#endif /* TIME_STAMP_H_ */
