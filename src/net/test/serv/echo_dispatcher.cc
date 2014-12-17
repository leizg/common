#include "echo_dispatcher.h"
#include "io/input_buf.h"
#include "io/connection.h"

namespace test {

void EchoDispatcher::dispatch(io::Connection* conn, io::InputBuf* input_buf,
                              const TimeStamp& time_stamp) {
  LOG(INFO)<<"recv data: " << time_stamp.microSecs();
}

}
