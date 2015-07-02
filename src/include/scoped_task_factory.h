#pragma once

#include "base/base.h"

// not threadsafe.
class ScopedTaskFactory {
  public:
    virtual ~ScopedTaskFactory() {
      mark();
    }

    class Flag : public RefCounted {
      public:
        Flag()
            : cancel_(false) {
        }
        virtual ~Flag() {
        }

        bool cancel() const {
          return !cancel_;
        }
        void mark(bool is_cacnel) {
          cancel_ = is_cacnel;
        }

      private:
        bool cancel_;

        DISALLOW_COPY_AND_ASSIGN(Flag);
    };

    class ScopedTask : public Closure {
      public:
        virtual ~ScopedTask() {
        }

        bool cancel() {
          return flag_->cancel();
        }

      protected:
        ScopedTask(ScopedTaskFactory* factory) {
          DCHECK_NOTNULL(factory);
          flag_(factory->flag_.get());
          flag_->Ref();
        }

        scoped_ref<Flag> flag_;
    };

    void mark(bool cacnel = true) {
      flag_->mark(cacnel);
    }

  protected:
    ScopedTaskFactory() {
      flag_.reset(new Flag);
    }

  private:
    scoped_ref<Flag> flag_;

    DISALLOW_COPY_AND_ASSIGN(ScopedTaskFactory);
};
