#ifndef OBJECT_SAVER_H_
#define OBJECT_SAVER_H_

#include "base/base.h"

template<typename Key, typename Object>
class ObjectSaver {
  public:
    virtual ~ObjectSaver() {
      if (check_empty_) {
//        DCHECK(empty());
      }
    }

    uint64 size() const {
      return size_;
    }
    bool empty() const {
      return size() == 0;
    }
    virtual void clear() = 0;

    virtual Object* Find(const Key& key) const = 0;

    virtual bool Add(const Key& key, Object* obj) = 0;
    // return false iif key is not exist.
    virtual bool Remove(const Key& key) = 0;

  protected:
    uint64 size_;

    explicit ObjectSaver(bool check_empty)
        : size_(0), check_empty_(check_empty) {
    }

  private:
    const bool check_empty_;

    DISALLOW_COPY_AND_ASSIGN(ObjectSaver);
};

template<typename Key, typename Object>
class ObjectMapSaver : public ObjectSaver<Key, Object> {
  protected:
    typedef std::map<Key, Object*> ObjectMap;
    typedef ObjectSaver<Key, Object> SaverType;

    ObjectMap object_map_;

  public:
    explicit ObjectMapSaver(bool check_empty = true)
        : SaverType(check_empty) {
    }
    virtual ~ObjectMapSaver() {
    }

    virtual Object* Find(const Key& key) const {
      auto it = object_map_.find(key);
      if (it != object_map_.end()) {
        return it->second;
      }
      return NULL;
    }

    virtual bool Add(const Key& key, Object* obj) {
      if (object_map_.count(key) == 0) {
        object_map_[key] = obj;
        ++SaverType::size_;
        return true;
      }
      return false;
    }

    virtual bool Remove(const Key& key) {
      auto it = object_map_.find(key);
      if (it != object_map_.end()) {
        object_map_.erase(it);
        --SaverType::size_;
        return true;
      }
      return false;
    }

    virtual void clear() {
      STLMapClear(&object_map_);
      SaverType::size_ = 0;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ObjectMapSaver);
};

template<typename Key, typename Object>
class ObjectVectorSaver : public ObjectSaver<Key, Object> {
  protected:
    typedef std::vector<Object*> ObjectVector;
    typedef ObjectSaver<Key, Object> SaverType;

    ObjectVector object_vector_;

  public:
    explicit ObjectVectorSaver(bool check_empty = true)
        : SaverType(check_empty) {
    }
    virtual ~ObjectVectorSaver() {
    }

    virtual Object* Find(const Key& key) const {
      if (object_vector_.size() > key) {
        return object_vector_[key];
      }
      return NULL;
    }

    virtual bool Add(const Key& key, Object* obj) {
      if (object_vector_.size() <= key) {
        object_vector_.resize(key);
      }
      auto o = object_vector_[key];
      if (o != NULL) return false;

      ++SaverType::size_;
      object_vector_[key] = obj;
      return true;
    }

    virtual bool Remove(const Key& key) {
      bool flag = false;
      if (object_vector_.size() >= key) {
        auto it = object_vector_[key];
        if (it != NULL) {
          --SaverType::size_;
          flag = true;
          object_vector_[key] = NULL;
        }
      }

      // release buffer.
      if (SaverType::size_ == 0) {
        ObjectVector tmp;
        object_vector_.swap(tmp);
      }
      return flag;
    }

    virtual void clear() {
      SaverType::size_ = 0;
      for (auto i = SaverType::object_vector_.begin();
          i != SaverType::object_vector_.end(); ++i) {
        delete (*i);
      }
      SaverType::object_vector_.clear();
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ObjectVectorSaver);
};

template<typename Key, typename Object>
class RefCountedObjectMapSaver : public ObjectMapSaver<Key, Object> {
  protected:
    typedef ObjectMapSaver<Key, Object> SaverType;

  public:
    explicit RefCountedObjectMapSaver(bool check_empty = true)
        : SaverType(check_empty) {
    }
    virtual ~RefCountedObjectMapSaver() {
    }

    virtual Object* Find(const Key& key) const {
      Object* obj = SaverType::Find(key);
      if (obj != NULL) {
        obj->Ref();
        return obj;
      }
      return NULL;
    }

    virtual bool Add(const Key& key, Object* obj) {
      if (SaverType::Add(key, obj)) {
        obj->Ref();
        return true;
      }
      return false;
    }
    virtual bool Remove(const Key& key) {
      auto it = SaverType::object_map_.find(key);
      if (it != SaverType::object_map_.end()) {
        SaverType::object_map_.erase(it);
        --SaverType::size_;
        it->second->UnRef();
        return true;
      }
      return false;
    }

    virtual void clear() {
      for (auto i = SaverType::object_map_.begin();
          i != SaverType::object_map_.end(); ++i) {
        i->second->UnRef();
      }
      SaverType::object_map_.clear();

      SaverType::size_ = 0;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(RefCountedObjectMapSaver);
};

template<typename Key, typename Object>
class RefCountedObjectArraySaver : public ObjectVectorSaver<Key, Object> {
  protected:
    typedef ObjectVectorSaver<Key, Object> SaverType;

  public:
    explicit RefCountedObjectArraySaver(bool check_empty = true)
        : SaverType(check_empty) {
    }
    virtual ~RefCountedObjectArraySaver() {
    }

    virtual Object* Find(const Key& key) const {
      Object* obj = SaverType::Find(key);
      if (obj != NULL) {
        obj->Ref();
        return obj;
      }
      return NULL;
    }

    virtual bool Add(const Key& key, Object* obj) {
      if (SaverType::Add(key, obj)) {
        obj->Ref();
        return true;
      }
      return false;
    }
    virtual bool Remove(const Key& key) {
      bool flag = false;
      if (SaverType::object_vector_.size() >= key) {
        auto it = SaverType::object_vector_[key];
        if (it != NULL) {
          --SaverType::size_;
          flag = true;
          it->UnRef();
          SaverType::object_vector_[key] = NULL;
        }
      }

      // release buffer.
      if (SaverType::size_ == 0) {
        SaverType tmp;
        SaverType::object_vector_.swap(tmp);
      }
      return flag;
    }

    virtual void clear() {
      STLUnRef(&SaverType::object_vector_);
      SaverType::size_ = 0;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(RefCountedObjectArraySaver);
};

template<typename Key, typename Object,
    template<typename, typename > class Saver>
class ThreadSafeObjectSaver : public ObjectSaver<Key, Object> {
  protected:
    typedef ObjectSaver<Key, Object> SaverType;

    Saver<Key, Object> saver_;
    mutable RwLock rw_lock_;

  public:
    explicit ThreadSafeObjectSaver(bool check_empty = true)
        : SaverType(check_empty) {
    }
    virtual ~ThreadSafeObjectSaver() {
    }

    uint64 size() const {
      ScopedReadLock l(&rw_lock_);
      return saver_.size();
    }

    virtual Object* Find(const Key& key) const {
      ScopedReadLock l(&rw_lock_);
      return saver_.Find(key);
    }

    virtual bool Add(const Key& key, Object* obj) {
      ScopedWriteLock l(&rw_lock_);
      if (saver_.Add(key, obj)) {
        ++SaverType::size_;
        return true;
      }
      return false;
    }
    virtual bool Remove(const Key& key) {
      ScopedWriteLock l(&rw_lock_);
      if (saver_.Remove(key)) {
        --SaverType::size_;
        return true;
      }
      return false;
    }
    virtual void clear() {
      ScopedWriteLock l(&rw_lock_);
      --SaverType::size_ = 0;
      saver_.clear();
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ThreadSafeObjectSaver);
};

namespace internal {
template<typename Key>
class SplitFunctor {
  public:
    uint32 operator()(const Key& key) const {
      return static_cast<uint32>(key);
    }
};

template<>
class SplitFunctor<std::string> {
  public:
    uint32 operator()(const std::string& key) const {
      return Hash(key);
    }
};
template<>
class SplitFunctor<char*> {
  public:
    uint32 operator()(const char*& key) const {
      return Hash(key, ::strlen(key));
    }
};
}

template<typename Key, typename Object, typename Table>
class MulityTableObjectSaver : public ObjectSaver<Key, Object> {
  protected:
    typedef ObjectSaver<Key, Object> SaverType;

    std::vector<Table*> tables_;

  public:
    MulityTableObjectSaver(uint32 capacity, bool check_empty = true)
        : ObjectSaver<Key, Object>(check_empty), capacity_(capacity) {
      DCHECK_GT(capacity, 0);
      DCHECK_EQ(capacity % 2, 0);

      tables_.resize(capacity);
      for (uint32 i = 0; i < capacity; ++i) {
        tables_[i] = new Table;
      }
    }
    virtual ~MulityTableObjectSaver() {
      STLClear(&tables_);
    }

    virtual Object* Find(const Key& key) const {
      return getTable(key)->Find(key);
    }

    virtual bool Add(const Key& key, Object* obj) {
      Table* t = getTable(key);
      if (t->Add(key, obj)) {
        ++SaverType::size_;
        return true;
      }
      return false;
    }
    virtual bool Remove(const Key& key) {
      if (getTable(key)->Remove(key)) {
        --SaverType::size_;
        return true;
      }
      return false;
    }

    virtual void clear() {
      SaverType::size_ = 0;
      for (uint32 i = 0; i < capacity_; ++i) {
        Table* t = tables_[i];
        t->clear();
      }
    }

  private:
    const uint32 capacity_;
    internal::SplitFunctor<Key> split_;

    Table* getTable(const Key& key) const {
      uint32 hash = split_(key);
      return tables_[hash & (capacity_ - 1)];
    }

    DISALLOW_COPY_AND_ASSIGN(MulityTableObjectSaver);
};

#endif /* OBJECT_SAVER_H_ */
