#include "timer_list.h"

namespace io {

TimeStamp TimerListImpl::insert(const TimeStamp& time_stamp, Closure* cb) {
  EntryList::iterator pos;
  for (pos = entries_.begin(); pos != entries_.end(); ++pos) {
    const TimeStamp& ts = (*pos).first;
    if (!(ts < time_stamp)) {
      break;
    }
  }

  entries_.insert(pos, std::make_pair(time_stamp, cb));
  DCHECK(!entries_.empty());
  return entries_.begin()->first;
}

bool TimerListImpl::fireActivedTimer(const TimeStamp& time_stamp,
                                     TimeStamp* next_expired) {
  while (!entries_.empty()) {
    const Entry& entry = *entries_.begin();
    if (entry.first > time_stamp) {
      break;
    }

    Closure* cb = entry.second;
    AutoRunner<Closure> r(cb);
    entries_.pop_front();
  }

  if (!entries_.empty()) {
    *next_expired = entries_.begin()->first;
    return true;
  }
  return false;
}

}
