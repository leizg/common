#ifndef BASE_H_
#define BASE_H_

#include "hash.h"
#include "closure.h"
#include "disk_file.h"
#include "macro_def.h"
#include "scoped_ptr.h"
#include "scoped_ref.h"
#include "data_types.h"

#include "thread.h"
#include "thread_util.h"

#include <time.h>
#include <unistd.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <set>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>

#define SetFdBlock(fd) \
  ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK)
#define SetFdNonBlock(fd) \
  ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) & ~O_NONBLOCK)

// stl utilites
template<typename T> inline void STLClear(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    delete *it;
  }
  t->clear();
}
template<typename T> inline void MapClear(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    delete it->second;
  }
  t->clear();
}

template<typename T> inline void STLUnRef(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    (*it)->Ref();
  }
  t->clear();
}
template<typename T> inline void MapUnRef(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    (it->second)->UnRef();
  }
  t->clear();
}

// not held the closure.
template<typename ClosureType>
class AutoRunner {
  public:
    explicit AutoRunner(ClosureType* closure)
        : closure_(closure) {
    }
    ~AutoRunner() {
      if (closure_ != NULL) {
        closure_->Run();
      }
    }

    ClosureType* release() {
      ClosureType* tmp = closure_;
      closure_ = NULL;
      return tmp;
    }

  private:
    ClosureType* closure_;

    DISALLOW_COPY_AND_ASSIGN(AutoRunner);
};

#endif
