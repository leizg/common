#ifndef GZIP_H_
#define GZIP_H_

#include "compression.h"
#include <zlib.h>

namespace util {

class GzipCompression : public Compression {
  public:
    GzipCompression() {
    }
    virtual ~GzipCompression() {
    }

    virtual bool compress(InBuf* in_buf, OutBuf* out_buf);
    virtual bool decompress(InBuf* in_buf, OutBuf* out_buf);

  private:
    DISALLOW_COPY_AND_ASSIGN (GzipCompression);
};
}

#endif
