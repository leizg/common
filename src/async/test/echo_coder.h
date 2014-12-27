#ifndef ECHO_CODER_H_
#define ECHO_CODER_H_

#include "base/base.h"

namespace io {
class InputStream;
class OutputStream;
}

namespace test {

bool Encode(const char* data_buf, uint32 len, io::OutputStream* buf);
bool Decode(io::InputStream* buf, bool* is_last, uint32* data_len);

}
#endif /* ECHO_CODER_H_ */
