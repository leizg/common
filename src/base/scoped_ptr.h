#pragma once

#include <assert.h>
#include <stdlib.h>

// copy from google protobuf.
template<class C>
class scoped_ptr {
  public:
    // The element type
    typedef C element_type;

    // Constructor.  Defaults to intializing with NULL.
    // There is no way to create an uninitialized scoped_ptr.
    // The input parameter must be allocated with new.
    explicit scoped_ptr(C* p = NULL)
        : ptr_(p) {
    }

    // Destructor.  If there is a C object, delete it.
    // We don't need to test ptr_ == NULL because C++ does that for us.
    ~scoped_ptr() {
      enum {
        type_must_be_complete = sizeof(C)
      };
      delete ptr_;
    }

    // Reset.  Deletes the current owned object, if any.
    // Then takes ownership of a new object, if given.
    // this->reset(this->get()) works.
    void reset(C* p = NULL) {
      if (p != ptr_) {
        enum {
          type_must_be_complete = sizeof(C)
        };
        delete ptr_;
        ptr_ = p;
      }
    }

    // Accessors to get the owned object.
    // operator* and operator-> will assert() if there is no current object.
    C& operator*() const {
      assert(ptr_ != NULL);
      return *ptr_;
    }
    C* operator->() const {
      assert(ptr_ != NULL);
      return ptr_;
    }
    C* get() const {
      return ptr_;
    }

    // Comparison operators.
    // These return whether two scoped_ptr refer to the same object, not just to
    // two different but equal objects.
    bool operator==(C* p) const {
      return ptr_ == p;
    }
    bool operator!=(C* p) const {
      return ptr_ != p;
    }

    // Swap two scoped pointers.
    void swap(scoped_ptr& p2) {
      C* tmp = ptr_;
      ptr_ = p2.ptr_;
      p2.ptr_ = tmp;
    }

    // Release a pointer.
    // The return value is the current pointer held by this object.
    // If this object holds a NULL pointer, the return value is NULL.
    // After this operation, this object will hold a NULL pointer,
    // and will not own the object any more.
    C* release() {
      C* retVal = ptr_;
      ptr_ = NULL;
      return retVal;
    }

  private:
    C* ptr_;

    // Forbid comparison of scoped_ptr types.  If C2 != C, it totally doesn't
    // make sense, and if C2 == C, it still doesn't make sense because you should
    // never have the same object owned by two different scoped_ptrs.
    template<class C2> bool operator==(scoped_ptr<C2> const& p2) const;
    template<class C2> bool operator!=(scoped_ptr<C2> const& p2) const;

    // Disallow evil constructors
    scoped_ptr(const scoped_ptr&);
    void operator=(const scoped_ptr&);
};

// scoped_array<C> is like scoped_ptr<C>, except that the caller must allocate
// with new [] and the destructor deletes objects with delete [].
//
// As with scoped_ptr<C>, a scoped_array<C> either points to an object
// or is NULL.  A scoped_array<C> owns the object that it points to.
//
// Size: sizeof(scoped_array<C>) == sizeof(C*)
template<class C>
class scoped_array {
  public:

    // The element type
    typedef C element_type;

    // Constructor.  Defaults to intializing with NULL.
    // There is no way to create an uninitialized scoped_array.
    // The input parameter must be allocated with new [].
    explicit scoped_array(C* p = NULL)
        : array_(p) {
    }

    // Destructor.  If there is a C object, delete it.
    // We don't need to test ptr_ == NULL because C++ does that for us.
    ~scoped_array() {
      enum {
        type_must_be_complete = sizeof(C)
      };
      delete[] array_;
    }

    // Reset.  Deletes the current owned object, if any.
    // Then takes ownership of a new object, if given.
    // this->reset(this->get()) works.
    void reset(C* p = NULL) {
      if (p != array_) {
        enum {
          type_must_be_complete = sizeof(C)
        };
        delete[] array_;
        array_ = p;
      }
    }

    // Get one element of the current object.
    // Will assert() if there is no current object, or index i is negative.
    C& operator[](size_t i) const {
      assert(array_ != NULL);
      return array_[i];
    }

    // Get a pointer to the zeroth element of the current object.
    // If there is no current object, return NULL.
    C* get() const {
      return array_;
    }

    // Comparison operators.
    // These return whether two scoped_array refer to the same object, not just to
    // two different but equal objects.
    bool operator==(C* p) const {
      return array_ == p;
    }
    bool operator!=(C* p) const {
      return array_ != p;
    }

    // Swap two scoped arrays.
    void swap(scoped_array& p2) {
      C* tmp = array_;
      array_ = p2.array_;
      p2.array_ = tmp;
    }

    // Release an array.
    // The return value is the current pointer held by this object.
    // If this object holds a NULL pointer, the return value is NULL.
    // After this operation, this object will hold a NULL pointer,
    // and will not own the object any more.
    C* release() {
      C* retVal = array_;
      array_ = NULL;
      return retVal;
    }

  private:
    C* array_;

    // Forbid comparison of different scoped_array types.
    template<class C2> bool operator==(scoped_array<C2> const& p2) const;
    template<class C2> bool operator!=(scoped_array<C2> const& p2) const;

    // Disallow evil constructors
    scoped_array(const scoped_array&);
    void operator=(const scoped_array&);
};
