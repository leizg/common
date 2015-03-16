#include "echo_coder.h"

#define LAST_GUARD (1UL << 31)

namespace test {

#if 0
bool Encode(const char* data_buf, uint32 len, io::OutputBuf* buf) {
  uint32 hdr = htonl(len | LAST_GUARD);
  DLOG(INFO)<< "encode hdr: " << hdr;

  char* data;
  int size = sizeof(uint32);
  buf->Next(&data, &size);
  if (size != sizeof(uint32)) {
    LOG(WARNING)<< "encode header error";
    return false;
  }
  ::memcpy(data, &hdr, sizeof(hdr));

  size = len;
  buf->Next(&data, &size);
  if (size != len) {
    LOG(WARNING)<< "encode data error";
    return false;
  }
  ::memcpy(data, data_buf, size);

  return true;
}
#endif

bool Decode(const char* buf, bool* is_last, uint32* data_len) {
  uint32 val = 0;
  ::memcpy(&val, buf, sizeof(uint32));
  uint32 hdr = ntohl(val);
  DLOG(INFO)<< "decode hdr: " << hdr;

  *is_last = hdr & LAST_GUARD;
  *data_len = hdr & (~LAST_GUARD);
  return true;
}

}

