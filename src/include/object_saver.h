#ifndef OBJECT_SAVER_H_
#define OBJECT_SAVER_H_

#include "base/base.h"

template<typename Object>
class ObjectSaver {
  public:
    class Policy {
      public:
        virtual ~Policy() {
        }

        virtual void Ref(Object* object) const = 0;
        virtual void UnRef(Object* object) const = 0;
    };

    virtual ~ObjectSaver() {
    }

    virtual bool Add(Object* obj) = 0;
    virtual void Remove(Object* Obj) = 0;

  protected:
    explicit ObjectSaver(Policy* delegate)
        : delegate_(delegate) {
      DCHECK_NOTNULL(delegate);
    }

    scoped_ptr<Policy> delegate_;

  private:
    DISALLOW_COPY_AND_ASSIGN(ObjectSaver);
};

template<typename Key, typename Object>
class ObjectMapSaver : ObjectSaver<Object> {
  public:
    typedef typename ObjectSaver<Object>::Policy Policy;

    explicit ObjectMapSaver(Policy* policy)
        : ObjectSaver<Object>(policy) {
    }
    virtual ~ObjectMapSaver() {
      DCHECK(object_map_.empty());
    }

    Object* Find(Key key) const {
      ScopedMutex l(&mutex_);
      auto it = object_map_.find(key);
      if (it != object_map_.end()) {
        auto obj = it->second;
        ObjectSaver<Object>::delegate_->Ref(obj);
        return obj;
      }
      return NULL;
    }

    virtual bool Add(Object* obj) {
      ScopedMutex l(&mutex_);
      Key k = obj->Key();
      if (object_map_.count(k) == 0) {
        ObjectSaver<Object>::delegate_->Ref(obj);
        object_map_[k] = obj;
        return true;
      }
      return false;
    }
    void Remove(Object* obj) {
      ScopedMutex l(&mutex_);
      Key k = obj->Key();
      auto it = object_map_.find(k);
      if (it != object_map_.end()) {
        ObjectSaver<Object>::delegate_->UnRef(obj);
        object_map_.erase(it);
      }
    }

  protected:
    typedef std::map<Key, Object*> ObjectMap;
    mutable Mutex mutex_;
    ObjectMap object_map_;

  private:

    DISALLOW_COPY_AND_ASSIGN(ObjectMapSaver);
};

template<typename Key, typename Object>
class ObjectVectorSaver : ObjectSaver<Object> {
  public:
    typedef typename ObjectSaver<Object>::Policy Policy;

    explicit ObjectVectorSaver(Policy* policy)
        : ObjectSaver<Object>(policy) {
    }
    virtual ~ObjectVectorSaver() {
      DCHECK(object_vector_.empty());
    }

    Object* Find(Key key) const {
      ScopedMutex l(&mutex_);
      if (object_vector_.size() <= key) {
        return NULL;
      }

      auto it = object_vector_[key];
      if (it != NULL) {
        ObjectSaver<Object>::delegate_->Ref(it);
        return it;
      }
      return NULL;
    }

    virtual bool Add(Object* obj) {
      ScopedMutex l(&mutex_);
      Key k = obj->Key();
      if (object_vector_.size() <= k) {
        object_vector_.reserve(k);
      }

      auto o = object_vector_[k];
      if (o != NULL) return false;

      ObjectSaver<Object>::delegate_->Ref(obj);
      object_vector_[k] = obj;
      return true;
    }

    virtual void Remove(Object* obj) {
      ScopedMutex l(&mutex_);
      Key k = obj->Key();
      if (object_vector_.size() >= k) {
        auto it = object_vector_[k];
        if (it != NULL) {
          ObjectSaver<Object>::delegate_->UnRef(obj);
          object_vector_[k] = NULL;
        }
      }
    }

  protected:
    typedef std::vector<Key, Object*> ObjectVector;
    mutable Mutex mutex_;
    ObjectVector object_vector_;

  private:
    DISALLOW_COPY_AND_ASSIGN(ObjectVectorSaver);
};

// do nothing.
template<typename Object>
class DefaultObjectSavePolicy : public ObjectSaver<Object>::Policy {
  public:
    DefaultObjectSavePolicy() {
    }
    virtual ~DefaultObjectSavePolicy() {
    }

  private:
    virtual void Ref(Object*) const {
    }
    virtual void UnRef(Object*) const {
    }

    DISALLOW_COPY_AND_ASSIGN(DefaultObjectSavePolicy);
};

template<typename Object>
class RefCountedObjectSavePolicy : public ObjectSaver<Object>::Policy {
  public:
    RefCountedObjectSavePolicy() {
    }
    virtual ~RefCountedObjectSavePolicy() {
    }

  private:
    virtual void Ref(Object* obj) const {
      obj->Ref();
    }
    virtual void UnRef(Object* obj) const {
      obj->UnRef();
    }

    DISALLOW_COPY_AND_ASSIGN(RefCountedObjectSavePolicy);
};

#endif /* OBJECT_SAVER_H_ */
