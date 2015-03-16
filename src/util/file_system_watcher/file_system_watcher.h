#ifndef DIR_WATCHER_H_
#define DIR_WATCHER_H_

#include "base/base.h"

namespace async {
struct Event;
class EventManager;
}

namespace util {
class WatchObject;

class FileSystemWatcher {
  public:
    explicit FileSystemWatcher(async::EventManager* ev_mgr);
    virtual ~FileSystemWatcher();

    bool Init();

    // return -1 iif error orrcured.
    // else return id of watched object.
    int watch(WatchObject* watcher);
    void rmWatch(int id);

    void handleKernelEvent();

  private:
    int fd_;

    async::EventManager* ev_mgr_;
    scoped_ptr<async::Event> event_;

    typedef std::map<int, WatchObject*> Map;
    Map map_;

    const static uint32 kBufSize = 4096;
    std::vector<char> buf_;
    std::string name_cache_;

    void stopWatch();
    int readEvents();

    DISALLOW_COPY_AND_ASSIGN(FileSystemWatcher);
};
}
#endif /* DIR_WATCHER_H_ */
