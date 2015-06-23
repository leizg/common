#pragma once

#include <stddef.h>
#include "macro_def.h"

template<typename Type>
class scoped_ref {
  public:
    scoped_ref(Type* ptr = NULL)
        : ptr_(ptr) {
    }
    ~scoped_ref() {
      if (ptr_ != NULL) {
        ptr_->UnRef();
      }
    }

    Type* get() const {
      return ptr_;
    }
    Type* release() {
      Type* ptr = ptr_;
      ptr_ = NULL;
      return ptr;
    }

    void reset(Type* ptr = NULL) {
      if (ptr_ != NULL) {
        ptr_->UnRef();
      }
      ptr_ = ptr;
    }

    Type* operator->() const {
      assert(ptr_ != NULL);
      return ptr_;
    }
    Type& operator*() {
      assert(ptr_ != NULL);
      return *ptr_;
    }

    bool operator ==(Type* ptr) const {
      return ptr_ == ptr;
    }
    bool operator !=(Type* ptr) const {
      return ptr_ != ptr;
    }

  private:
    Type* ptr_;

    DISALLOW_COPY_AND_ASSIGN(scoped_ref);
};

