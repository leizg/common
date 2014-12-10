#include "include/lru_cache.h"

#include <gtest/gtest.h>

namespace test {

class CacheValue : public RefCounted {
  public:
    CacheValue(uint64 id)
        : id_(id) {
    }

    uint64 key() const {
      return id_;
    }

  private:
    uint64 id_;

    virtual ~CacheValue() {
      DLOG(INFO)<< "release id: " << id_;
    }

    DISALLOW_COPY_AND_ASSIGN(CacheValue);
  };

class LruTest : public testing::Test {
  public:
    LruTest()
        : cache_(32) {
    }
    virtual ~LruTest() {
    }

  protected:
    typedef LruCache<uint64, CacheValue> Cache;
    Cache cache_;

    bool checkExist(uint64 id) {
      scoped_ref<CacheValue> c(cache_.find(id));
      return c != NULL;
    }

    bool addValue(uint64 id) {
      DCHECK(!checkExist(id));
      scoped_ref<CacheValue> c(new CacheValue(id));
      if (cache_.insert(id, c.get())) {
        DCHECK_EQ(c->RefCount(), 2);
        return true;
      }
      DLOG(WARNING)<< "addValue error, id: " << id;
      return false;
    }

    void removeValue(uint64 id) {
      scoped_ref<CacheValue> c(cache_.find(id));
      if (c != NULL) {
        DCHECK_EQ(c->RefCount(), 2);
        cache_.remove(id);
        DCHECK_EQ(c->RefCount(), 1);
        DCHECK(!checkExist(id));
      }
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(LruTest);
};

TEST_F(LruTest, addAndRemoveTest) {
  for (uint32 i = 0; i < 128; ++i) {
    ASSERT_TRUE(addValue(i));
  }
#if 0
  for (uint32 i = 0; i < 128; ++i) {
    EXPECT_TRUE(checkExist(i)) << i;
  }
#endif
  for (uint32 i = 0; i < 128; ++i) {
    removeValue(i);
  }
#if 0
  for (uint32 i = 0; i < 128; ++i) {
    EXPECT_TRUE(!checkExist(i));
  }
#endif
}

}
