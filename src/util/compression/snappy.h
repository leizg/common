#ifndef SNAPPY_H_
#define SNAPPY_H_

#include "compression.h"
#include <snappy.h>

namespace util {

class SnappyCompression : public Compression {
  public:
    SnappyCompression() {
    }
    virtual ~SnappyCompression() {
    }

  private:
    virtual bool compress(Context* ctx) const;
    virtual bool decompress(Context* ctx) const;

    virtual Context* newContext() const {
      Context* ctx = new Context;
      ctx->in_buf = nullptr;
      ctx->out_buf = nullptr;
      return ctx;
    }

    DISALLOW_COPY_AND_ASSIGN(SnappyCompression);
};

}
#endif
