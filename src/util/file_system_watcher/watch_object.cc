#include "watch_object.h"
#include "file_system_watcher.h"

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

void WatchObject::rmWatch() {
  if (watcher_ != NULL && watch_id_ != 0) {
    watcher_->rmWatch(watch_id_);
    watch_id_ = 0;
  }
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
    if (*fname == "." || *fname == "..") {
      continue;
    }

    std::string full_path(path_ + '/' + *fname);
    w.reset(new DirWatcher(full_path, watcher_, recurision_));
    if (w != NULL && !w->watch(mode, delegate)) {
      return false;
    }
  }

  return true;
}

}
