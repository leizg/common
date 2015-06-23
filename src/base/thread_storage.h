#pragma once

#include <pthread.h>
#include "macro_def.h"

template<typename ObjectType>
class ThreadStorage {
  public:
    ThreadStorage() {
      ::pthread_key_create(&key_, NULL);
      set(nullptr);
    }
    ~ThreadStorage() {
      ::pthread_key_delete(key_);
    }

    ObjectType* get() const {
      return static_cast<ObjectType*>(::pthread_getspecific(key_));
    }
    void set(ObjectType* obj) {
      ::pthread_setspecific(key_, obj);
    }

  private:
    pthread_key_t key_;

    DISALLOW_COPY_AND_ASSIGN(ThreadStorage);
};

