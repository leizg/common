#include "echo_coder.h"

#include "io/input_buf.h"
#include "io/output_buf.h"

#define LAST_GUARD (1 << 31)

namespace test {

bool EchoCoder::Encode(const char* data_buf, uint32 len, io::OutputBuf* buf) {
  uint32 header = len | LAST_GUARD;

  char* data;
  int size = sizeof(uint32);
  buf->Next(&data, &size);
  if (size != sizeof(uint32)) {
    LOG(WARNING)<< "encode header error";
    return false;
  }
  ::memcpy(data, &header, sizeof(header));

  size = len;
  buf->Next(&data, &size);
  if (size != len) {
    LOG(WARNING)<< "encode data error";
    return false;
  }
  ::memcpy(data, data_buf, len);

  return true;
}

bool EchoCoder::Decode(io::InputBuf* buf, bool* is_last, uint32* data_len) {
  int len = sizeof(uint32);
  const char* data;
  if (!buf->Next(&data, &len)) {
    LOG(WARNING)<< "decode header error";
    return false;
  }
  if (len != sizeof(uint32)) {
    LOG(WARNING)<< "header is too short";
    return false;
  }

  uint32 val = 0;
  ::memcpy(&val, data, sizeof(uint32));
  val = ntohl(val);

  *is_last = val & LAST_GUARD;
  *data_len = val & (~LAST_GUARD);
  return true;
}

}

