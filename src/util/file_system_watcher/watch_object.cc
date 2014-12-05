#include "watch_object.h"

namespace util {

bool WatchObject::watch(uint32 mode, Delegate* delegate) {
  if (delegate_ != NULL) return false;

  int wd = watcher_->watch(this);
  if (wd != -1) {
    mode_ = mode;
    delegate_ = delegate;
    watch_id_ = wd;

    return true;
  }

  return false;
}

bool DirWatcher::watch(uint32 mode, Delegate* delegate) {
  if (!WatchObject::watch(mode, delegate)) return false;
  if (!recurision_) return true;

  DirIterator it(path_);
  if (!it.Init()) return false;

  scoped_ref<WatchObject> w;
  const std::string* fname = it.next(DirIterator::DIR_TYPE);
  for (; fname != NULL; fname = it.next(DirIterator::DIR_TYPE)) {
    // FIXME: maybe skip file that start with '.'.
    if (fname == "." || fname == "..") {
      continue;
    }

    std::string full_path(path_ + '/' + *fname);
    w.reset(new DirWatcher(watcher_, full_path, recurision_));
    if (w != NULL && !w->watch(mode, delegate)) {
      return false;
    }
  }

  return true;
}

}
