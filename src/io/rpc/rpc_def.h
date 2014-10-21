#ifndef RPC_DEF_H_
#define RPC_DEF_H_

#include <google/protobuf/service.h>
#include <google/protobuf/message.h>

typedef google::protobuf::Service Service;
typedef google::protobuf::Message Message;
typedef google::protobuf::RpcController RpcController;
typedef google::protobuf::MethodDescriptor MethodDescriptor;

enum RpcType {
  RPC_REQUEST = 0, RPC_RESPONSE,
};

// fixed length: 16.
struct MessageHeader {
    uint32 fun_id;
    uint16 rpc_type;  // request or response.
    uint16 length;
    uint64 id;
}__attribute__((packed));

#define RPC_HEADER_LENGTH (sizeof (MessageHeader))

#endif /* RPC_DEF_H_ */
