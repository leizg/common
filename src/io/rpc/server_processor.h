#ifndef SERVER_PROCESSOR_H_
#define SERVER_PROCESSOR_H_

#include "io/protocol.h"

namespace rpc {
class HandlerMap;

class ServerProcessor : public io::Protocol::Processor {
  public:
    explicit ServerProcessor(HandlerMap* handler_map)
        : handler_map_(handler_map) {
      CHECK_NOTNULL(handler_map);
    }

    virtual ~ServerProcessor();

  private:
    HandlerMap* handler_map_;

    virtual void Dispatch(io::Connection* conn, io::InputBuf* input_buf,
                          const TimeStamp& time_stamp);

    DISALLOW_COPY_AND_ASSIGN(ServerProcessor);
};

}

#endif /* SERVER_PROCESSOR_H_ */
