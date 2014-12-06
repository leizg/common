#include "snappy.h"

namespace util {

bool SnappyCompression::compress(InBuf* in_buf, OutBuf* out_buf) {
  uint64 used_size = 0, max_size = snappy::MaxCompressedLength(in_buf->size());
  char* data;
  out_buf->ensureLeft(max_size);
  out_buf->write(&data, max_size);

  char* src_data;
  size_t src_len, dst_len;
  while (in_buf->read(&src_data, &src_len)) {
    snappy::RawCompress(src_data, src_len, data, &dst_len);
    data += dst_len;
    used_size += dst_len;
  }

  out_buf->backUp(max_size - used_size);
  return true;
}

bool SnappyCompression::decompress(InBuf* in_buf, OutBuf* out_buf) {
  char* src_data, *dst_data;
  size_t src_len, dst_len;
  while (in_buf->read(&src_data, &src_len)) {
    if (!snappy::GetUncompressedLength(src_data, src_len, &dst_len)) {
      LOG(WARNING)<< "decode compressed length error";
      return false;
    }

    out_buf->ensureLeft(dst_len);
    out_buf->write(&dst_data, dst_len);
    if (!snappy::RawUncompress(src_data, src_len, dst_data)) {
      LOG(WARNING) << "snappy uncompress error";
      return false;
    }
  }

  return true;
}

}
