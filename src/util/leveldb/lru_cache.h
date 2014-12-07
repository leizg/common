#ifndef LRU_CACHE_H_
#define LRU_CACHE_H_

#include "base/base.h"

namespace util {

// not threadsafe.
// Value must be RefCounted.
template<typename Key, typename Value>
class LruCache {
  public:
    explicit LruCache(uint32 size, bool check_empty = true)
        : size_(size), check_empty_(check_empty) {
      DCHECK_GT(size, 0);
    }
    ~LruCache() {
      if (check_empty_) {
        DCHECK(map_.empty());
      }
    }

    bool insert(const Key& k, Value* value);

    Value* find(const Key& k);

    void remove(const Key& k);
    void clear();

  private:
    uint32 size_;
    bool check_empty_;

    typedef std::map<Key, Value*> Map;
    Map map_;

    DISALLOW_COPY_AND_ASSIGN(LruCache);
};
}

#endif /* LRU_CACHE_H_ */
