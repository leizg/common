#include "echo_coder.h"
#include "io/memory_block.h"

#define LAST_GUARD (1UL << 31)

namespace test {

bool Encode(const char* data_buf, uint32 len, io::ExternableChunk* buf) {
  uint32 hdr = htonl(len | LAST_GUARD);
  DLOG(INFO)<< "encode hdr: " << hdr;
  ::memcpy(buf->peekW(), &hdr, sizeof(hdr));
  buf->skipWrite(sizeof(hdr));

  ::memcpy(buf->peekW(), data_buf, len);
  buf->skipWrite(len);

  return true;
}

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

