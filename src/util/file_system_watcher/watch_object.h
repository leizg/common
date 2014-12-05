#ifndef WATCH_OBJECT_H_
#define WATCH_OBJECT_H_

#include "base/base.h"

namespace util {
class FileSystemWatcher;

class WatchObject : public RefCounted {
  public:
    class Delegate {
      public:
        virtual ~Delegate() {
        }

        virtual void notify(WatchObject* object, uint32 event,
                            const std::string* path) const = 0;
    };

    enum FileType {
      REGULAR_FILE = 0, DIR_TYPE, SYMBOL_LINK,
    };

    WatchObject(FileType file_type, const std::string& path,
                FileSystemWatcher* watcher)
        : file_type_(file_type), path_(path), watch_id_(0), delegate_(NULL), watcher_(
            watcher), mode_(0) {
      DCHECK(!path.empty());
      DCHECK_NOTNULL(watcher);
    }
    virtual ~WatchObject() {
      rmWatch();
    }

    uint32 id() const {
      return watch_id_;
    }
    uint32 mode() const {
      return mode_;
    }

    FileType type() const {
      return file_type_;
    }
    const std::string& path() const {
      return path_;
    }

    virtual bool watch(uint32 mode, Delegate* delegate) = 0;
    void rmWatch() {
      if (watcher_ != NULL && watch_id_ != 0) {
        watcher_->rmWatch(watch_id_);
        watch_id_ = 0;
      }
    }

    void notify(uint32 event, const std::string* path) {
      delegate_->notify(this, event, path);
    }

  protected:
    const FileType file_type_;
    const std::string path_;

    uint32 mode_;
    int watch_id_;

    Delegate* delegate_;
    FileSystemWatcher* watcher_;

  private:
    DISALLOW_COPY_AND_ASSIGN(WatchObject);
};

class FileWatcher : public WatchObject {
  public:
    FileWatcher(const std::string& path, FileSystemWatcher* watcher)
        : WatchObject(REGULAR_FILE, path, watcher) {
    }
    virtual ~FileWatcher() {
    }

    virtual bool watch(uint32 mode, Delegate* delegate) {
      return WatchObject::watch(mode, delegate);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(FileWatcher);
};

class DirWatcher : public WatchObject {
  public:
    DirWatcher(const std::string& path, FileSystemWatcher* watcher,
               bool recurision)
        : WatchObject(REGULAR_FILE, path, watcher), recurision_(recurision) {
    }
    virtual ~DirWatcher() {
    }

    virtual bool watch(uint32 mode, Delegate* delegate);

  private:
    bool recurision_;

    DISALLOW_COPY_AND_ASSIGN(DirWatcher);
};

// FIXME: todo
class SymbolLinkWatcher : public WatchObject {
  public:
    SymbolLinkWatcher(const std::string& path, FileSystemWatcher* watcher,
                      bool follow_link)
        : WatchObject(REGULAR_FILE, path, watcher), follow_link_(follow_link) {
    }
    virtual ~SymbolLinkWatcher() {
    }

    virtual bool watch(uint32 mode, Delegate* delegate) {
      return WatchObject::watch(mode, delegate);
    }

  private:
    bool follow_link_;

    DISALLOW_COPY_AND_ASSIGN(SymbolLinkWatcher);
};
}
#endif /* WATCH_OBJECT_H_ */
