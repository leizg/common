#ifndef BASE_H_
#define BASE_H_

#include "closure.h"
#include "macro_def.h"
#include "scoped_ptr.h"
#include "scoped_ref.h"
#include "data_types.h"

#include "thread.h"
#include "thread_util.h"

// stl utilites
template<typename T> inline void STLClear(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    delete *it;
  }
  t->clear();
}
template<typename T> inline void MapClear(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    delete it->second;
  }
  t->clear();
}

template<typename T> inline void STLUnRef(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    (*it)->Ref();
  }
  t->clear();
}
template<typename T> inline void MapUnRef(T* t) {
  for (typename T::iterator it = t->begin(); it != t->end(); ++it) {
    (it->second)->UnRef();
  }
  t->clear();
}

#endif
