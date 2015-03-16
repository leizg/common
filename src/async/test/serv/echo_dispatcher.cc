#include "echo_dispatcher.h"

namespace test {

void EchoDispatcher::dispatch(async::Connection* conn,
                              io::InputStream* in_stream,
                              TimeStamp time_stamp) {
  LOG(INFO)<<"recv data: " << time_stamp.microSecs();
}

}
