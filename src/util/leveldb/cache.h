#ifndef CACHE_H_
#define CACHE_H_

#include "lru_cache.h"

namespace util {

class CacheObject : public RefCounted {
  public:
    uint64 id() const {
      return id_;
    }

  protected:
    uint64 id_;

    explicit CacheObject(uint64 id)
        : id_(id) {
    }

  private:
    // only should be deleted by UnRef().
    virtual ~CacheObject() {
    }

    DISALLOW_COPY_AND_ASSIGN(CacheObject);
};

class LevelDb;
class CacheManager {
  public:
    CacheManager(LevelDb* db, uint32 cache_size)
        : db_(db), lru_cache_(cache_size) {
      DCHECK_NOTNULL(db);
    }
    ~CacheManager();

    void flush();

    CacheObject* find(uint64 id);
    void addDirty(CacheObject* obj);
    void addDirty(std::list<CacheObject*>* obj_list);

    void remove(uint64 id);

  private:
    LevelDb* db_;

    typedef LruCache<uint64, CacheObject> Cache;
    Cache lru_cache_;

    DISALLOW_COPY_AND_ASSIGN(CacheManager);
};
}
#endif /* CACHE_H_ */
