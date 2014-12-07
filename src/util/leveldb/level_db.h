#ifndef LEVEL_DB_H_
#define LEVEL_DB_H_

#include <leveldb/db.h>
#include "base/base.h"

namespace util {

class LevelDb {
  public:
    LevelDb(const std::string& db_dir)
        : db_dir_(db_dir), flush_event_(false, false) {
    }
    ~LevelDb();

    // block_size: Approximate size of user data packed per block,
    //    0: means default size(4K)
    bool Init(bool create_if_missing, uint32 block_size = 0,
              bool enable_compression = false);

    void flush();

    bool get(const std::string& key, std::string* value);
    void put(const std::string* key, const std::string* value);

    void remove(const std::string* key);

  private:
    const std::string db_dir_;

    RwLock rw_lock_;
    scoped_ptr<leveldb::ReadOptions> read_opts_;
    scoped_ptr<leveldb::WriteOptions> write_opts_;
    scoped_ptr<leveldb::WriteBatch> write_batch_;

    scoped_ptr<leveldb::DB> db_;

    std::list<const std::string*> cache_list_;

    void flushInternal();
    Mutex mutex_;
    SyncEvent flush_event_;
    scoped_ptr<StoppableThread> flush_thread_;

};
}
#endif /* LEVEL_DB_H_ */
