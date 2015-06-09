#pragma once

// see also: https://github.com/idealvin/base/blob/master/closure.h

#include <tuple>
#include "macro_def.h"

template<int...> class Seq {};

template<int N, int ... S>
struct Indices : public Indices<N - 1, N - 1, S...> {
};

template<int ... S> struct Indices<0, S...> {
    typedef Seq<S...> SeqType;
};

template<typename Func, typename ... A, int ... S>
void apply(Func func, const std::tuple<A...>& args, Seq<S...>) {
  func(std::get<S>(args)...);
}

template<typename Func, typename ... A>
void apply(Func func, const std::tuple<A...>& args) {
  apply(func, args, typename Indices<sizeof...(A)>::SeqType());
}

template<typename Class, typename Func, typename ... A, int ... S>
void apply(Class* obj, Func func, const std::tuple<A...>& args, Seq<S...>) {
  (obj->*func)(std::get<S>(args)...);
}

template<typename Class, typename Func, typename ... A>
void apply(Class* obj, Func func, const std::tuple<A...>& args) {
  apply(obj, func, args, typename Indices<sizeof...(A)>::SeqType());
}

class Closure {
  public:
    virtual ~Closure() {
    }

    virtual void Run() = 0;
};

template<typename Func, typename ... A>
class FunctionClosure : public Closure {
  public:
    FunctionClosure(bool self_delete, Func func, A ... a)
        : func_(func), args_(a...) {
      self_delete_ = self_delete;
    }
    virtual ~FunctionClosure() {
    }

    virtual void Run() {
      bool self_delete = self_delete_;  // if delete self...
      apply(func_, args_);
      if (self_delete) delete this;
    }

  private:
    Func func_;
    std::tuple<A...> args_;

    bool self_delete_;
};

template<typename Class, typename Func, typename ...A>
class MethodClosure : public Closure {
  public:
    MethodClosure(bool self_delete, Class* obj, Func func, A ... a)
        : obj_(obj), func_(func), args_(a...) {
      self_delete_ = self_delete;
    }
    virtual ~MethodClosure() {
    }

    virtual void Run() {
      bool self_delete = self_delete_;  // if delete self...
      apply(obj_, func_, args_);
      if (self_delete) delete this;
    }

  private:
    Class* obj_;
    Func func_;
    std::tuple<A...> args_;

    bool self_delete_;
};

template<typename Func, typename ... A>
Closure* NewCallback(Func func, A ... a) {
  return new FunctionClosure<Func, A...>(true, func, a...);
}

template<typename Func, typename ... A>
Closure* NewPermanentCallback(Func func, A ... a) {
  return new FunctionClosure<Func, A...>(false, func, a...);
}

template<typename Class, typename Func, typename ... A>
Closure* NewCallback(Class* obj, Func func, A ... a) {
  return new MethodClosure<Class, Func, A...>(true, obj, func, a...);
}

template<typename Class, typename Func, typename ... A>
Closure* NewPermanentCallback(Class* obj, Func func, A ... a) {
  return new MethodClosure<Class, Func, A...>(false, obj, func, a...);
}

