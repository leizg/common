#include <sys/inotify.h>

#include "async/event_manager.h"

#include "watch_object.h"
#include "file_system_watcher.h"

namespace {

void handleWatcher(int fd, void* arg, uint8 event,
                   const TimeStamp& time_stamp) {
  util::FileSystemWatcher* w = static_cast<util::FileSystemWatcher*>(arg);
  w->handleKernelEvent();
}
}

namespace util {

FileSystemWatcher::FileSystemWatcher(async::EventManager* ev_mgr)
    : fd_(INVALID_FD), ev_mgr_(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);
}

FileSystemWatcher::~FileSystemWatcher() {
  stopWatch();
  DCHECK(map_.empty());
}

bool FileSystemWatcher::Init() {
  if (fd_ != INVALID_FD) return false;

#ifdef __linux__
  fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
#else
  fd_ = inotify_init();
#endif
  if (fd_ == -1) {
    PLOG(WARNING)<< "inotify_init1 error";
    return false;
  }

#ifndef __linux__
  setFdNonBlock(fd_);
  setFdCloExec(fd_);
#endif

  event_.reset(new async::Event);
  event_->fd = fd_;
  event_->event = EV_READ;
  event_->arg = this;
  event_->cb = handleWatcher;
  if (!ev_mgr_->Add(event_.get())) {
    closeWrapper(fd_);
    return false;
  }

  buf_.resize(kBufSize);
  return true;
}

int FileSystemWatcher::watch(WatchObject* watcher) {
  if (fd_ == INVALID_FD || watcher->id() != 0) return -1;

  const std::string& path = watcher->path();
  int wd = ::inotify_add_watch(fd_, path.c_str(),
                               watcher->mode() | IN_MASK_ADD);
  if (wd == -1) {
    PLOG(WARNING)<< "inotify_add_watch error";
    return false;
  }

  map_[wd] = watcher;
  watcher->Ref();

  return wd;
}

void FileSystemWatcher::rmWatch(int wd) {
  if (wd != 0 && map_.count(wd) != 0) {
    STLMapEarseAndUnRef(&map_, wd);

    int ret = ::inotify_rm_watch(fd_, wd);
    if (ret != 0) {
      PLOG(WARNING)<< "inotify_rm_watch error";
    }
  }
}

void FileSystemWatcher::stopWatch() {
  if (event_ != NULL) {
    ev_mgr_->Del(*event_);
  }

  // TODO: clear map_.

  closeWrapper(fd_);
}

int FileSystemWatcher::readEvents() {
  int readn = -1;

  while (true) {
    char* data = buf_.data();
    readn = ::read(fd_, data, kBufSize);
    if (readn == 0) {
      stopWatch();
      return 0;
    } else if (readn == -1) {
      switch (errno) {
        case EINTR:
          continue;
        case EINVAL:
        case EWOULDBLOCK:
          return 0;
        default:
          break;
      }
    }

    DCHECK_GT(readn, 0);
    return readn;
  }

  PLOG(WARNING)<< "read inotify event error";
  stopWatch();
  return readn;
}

void FileSystemWatcher::handleKernelEvent() {
  ev_mgr_->assertThreadSafe();

  int len = readEvents();
  char* data = buf_.data();

  struct inotify_event* event;
  while (len >= sizeof(*event)) {
    event = reinterpret_cast<struct inotify_event*>(data);
    data += sizeof(*event);
    len -= sizeof(*event);

    name_cache_.clear();
    int name_len = std::min(event->len, (uint32) len);
    if (name_len != 0) {
      name_cache_ = std::string(event->name, name_len);
      len -= name_len;
      data += name_len;
    }

    auto it = map_.find(event->wd);
    if (it != map_.end()) {
      WatchObject* w = it->second;
      w->notify(event->mask, &name_cache_);
    }
  }
}

}

