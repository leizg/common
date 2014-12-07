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

    virtual bool compress(InBuf* in_buf, OutBuf* out_buf);
    virtual bool decompress(InBuf* in_buf, OutBuf* out_buf);

  private:
    DISALLOW_COPY_AND_ASSIGN(SnappyCompression);
};

}
#endif
