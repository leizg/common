#ifndef ECHO_CODER_H_
#define ECHO_CODER_H_

#include "base/base.h"

namespace io {
class InputBuf;
class OutputBuf;
}

namespace test {

class EchoCoder {
  public:
    EchoCoder() {
    }

    bool Encode(const char* data_buf, uint32 len, io::OutputBuf* buf);
    bool Decode(io::InputBuf* buf, bool* is_last, uint32* data_len);

  private:
    DISALLOW_COPY_AND_ASSIGN(EchoCoder);
};
}
#endif /* ECHO_CODER_H_ */
