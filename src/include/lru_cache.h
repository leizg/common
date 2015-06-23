#pragma once

#include "include/link_queue.h"

namespace internal {

// todo: remove map_
template<typename Key, typename Value>
class Table : public LinkQueue {
  public:
    Table(uint64 capacity, bool check_empty = true)
        : capacity_(capacity), check_empty_(check_empty) {
      DCHECK_GT(capacity, 0);
    }
    virtual ~Table() {
      if (check_empty_) {
        DCHECK(map_.empty());
      }
    }

    void clear();

    bool remove(const Key& k);
    Value* find(const Key& k);
    bool insert(const Key& k, Value* value);

  private:
    uint64 capacity_;
    const bool check_empty_;

    class NodeValue : public LinkNode {
      public:
        NodeValue(const Key key, Value* value)
            : key_(key), value_(value) {
          DCHECK_NOTNULL(value);
          value->Ref();
        }
        virtual ~NodeValue() {
        }

        const Key& key() const {
          return key_;
        }
        Value* value() const {
          Value*v = value_.get();
          v->Ref();
          return v;
        }

      private:
        const Key key_;
        scoped_ref<Value> value_;

        DISALLOW_COPY_AND_ASSIGN(NodeValue);
    };

    typedef std::map<Key, NodeValue*> Map;
    Map map_;

    RwLock rw_lock_;

    DISALLOW_COPY_AND_ASSIGN(Table);
};
}

#define HASH_SHIFT      (4U)
#define HASH_SIZE       (1 << HASH_SHIFT)
#define HASH_MASK       (HASH_SIZE - 1)

// threadsafe.
// Key must be: uint32, uint64, char*, std::string.
// Value must be RefCounted.
template<typename Key, typename Value>
class LruCache {
  private:
    typedef internal::Table<Key, Value> HashTable;

  public:
    explicit LruCache(uint64 capacity, bool check_empty = true)
        : size_(0) {
      DCHECK_GT(capacity, 0);

      // build tables...
      uint64 capacity_per_table = capacity / HASH_SIZE;
      for (uint32 i = 0; i < HASH_SIZE; ++i) {
        HashTable * table = new HashTable(capacity_per_table, check_empty);
        tables_.push_back(table);
      }
    }
    ~LruCache() {
      clear();
      STLClear(&tables_);
    }

    bool insert(const Key& k, Value* value) {
      HashTable* t = getTableByKey(k);
      if (t->insert(k, value)) {
        ++size_;
        return true;
      }
      return false;
    }
    void remove(const Key& k) {
      HashTable* t = getTableByKey(k);
      if (t->remove(k)) {
        --size_;
      }
    }
    Value* find(const Key& k) {
      HashTable* t = getTableByKey(k);
      return t->find(k);
    }

    void clear() {
      size_ = 0;
      for (uint32 i = 0; i < HASH_SIZE; ++i) {
        HashTable * table = tables_[i];
        table->clear();
      }
    }

  private:
    uint32 size_;
    RwLock rw_lock_;

    HashTable* getTableByKey(const Key& k) {
      uint32 hash_id = Hash(k);
      return tables_[hash_id & HASH_MASK];
    }

    std::vector<HashTable*> tables_;

    DISALLOW_COPY_AND_ASSIGN(LruCache);
};

namespace internal {

template<typename Key, typename Value>
bool Table<Key, Value>::insert(const Key& k, Value* value) {
  ScopedWriteLock l(&rw_lock_);
  if (map_.count(k) != 0) return false;

  if (size_ == capacity_) {
    NodeValue* old_value = static_cast<NodeValue*>(prev);
    map_.erase(old_value->key());
    old_value->remove();
    delete old_value;
    --size_;
  }

  DCHECK_LT(size_, capacity_);
  NodeValue* new_value = new NodeValue(k, value);
  new_value->inertBefore(prev);
  map_[k] = new_value;
  ++size_;

  return true;
}

template<typename Key, typename Value>
Value* Table<Key, Value>::find(const Key& k) {
  ScopedReadLock l(&rw_lock_);
  auto it = map_.find(k);
  if (it != map_.end()) {
    NodeValue* v = it->second;
    v->remove();
    v->inertAfter(prev);
    return v->value();
  }

  return NULL;
}

template<typename Key, typename Value>
bool Table<Key, Value>::remove(const Key& k) {
  ScopedWriteLock l(&rw_lock_);
  auto it = map_.find(k);
  if (it != map_.end()) {
    map_.erase(it);
    NodeValue* v = it->second;
    v->remove();
    delete v;
    --size_;
    return true;
  }

  return false;
}

template<typename Key, typename Value>
void Table<Key, Value>::clear() {
  ScopedWriteLock l(&rw_lock_);
  prev = next = this;
  STLMapClear(&map_);

  size_ = 0;
}
}  // end for namespace internal

