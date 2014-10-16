#ifndef SCOPED_REF_H_
#define SCOPED_REF_H_

#include "macro_def.h"
#include "ref_counted.h"

#include <stddef.h>

template<typename Type>
class scoped_ref {
 public:
  scoped_ref(Type* ptr)
      : ptr_(ptr) {
  }
  ~scoped_ref() {
    ptr_->UnRef();
  }

  Type* get() const {
    return ptr_;
  }
  Type* release() {
    Type* ptr = ptr_;
    ptr_ = NULL;
    return ptr;
  }

  void reset(Type* ptr=NULL) {
    if (ptr_ != NULL) {
      ptr_->UnRef();
    }
    ptr_ = ptr;
  }

  Type* operator->() {
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

#endif
