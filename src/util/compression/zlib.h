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

  private:
    struct ZlibContext : public Compression::Context {
        ZlibContext() {
          status = Z_OK;
          in_buf = nullptr;
          out_buf = nullptr;

          ::memset(&stream, 0, sizeof stream);
          stream.next_in = nullptr;
          stream.next_out = nullptr;
          stream.msg = nullptr;
          stream.state = nullptr;
          stream.zalloc = Z_NULL;
          stream.zfree = Z_NULL;
          stream.opaque = nullptr;
        }

        int status;
        z_stream stream;
    };

    virtual Context* newContext() const {
      return new ZlibContext;
    }

    virtual bool compress(Context* ctx) const;
    virtual bool decompress(Context* ctx) const;

    DISALLOW_COPY_AND_ASSIGN (GzipCompression);
};
}

#endif
