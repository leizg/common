#include "rpc_def.h"
#include "zero_copy_stream.h"
#include "rpc_processor.h"

#if 0
#include "io/io_buf.h"
#include "io/input_buf.h"
#include "io/output_buf.h"
#include "io/connection.h"
#endif

namespace rpc {

void RpcProcessor::dispatch(io::Connection* conn, io::InputBuf* input_buf,
                            const TimeStamp& time_stamp) {
  const MessageHeader& header = GetRpcHeaderFromConnection(conn);
  if (IS_RESPONSE(header)) {
    reply_delegate_->process(conn, input_buf, time_stamp);
    return;
  }

  request_delegate_->process(conn, input_buf, time_stamp);
}

}
