#include "io/io_buf.h"
#include "io/input_buf.h"
#include "io/output_buf.h"
#include "io/connection.h"

#include "handler_map.h"
#include "rpc_processor.h"
#include "zero_copy_stream.h"
#include "rpc_request_handler.h"

namespace {

class ReplyObject : public io::OutVectorObject::IoObject {
  public:
    ReplyObject(const MessageHeader& header, Message* reply)
        : msg_(reply) {
      DCHECK_NOTNULL(reply);
      uint32 total_len = msg_->ByteSize() + RPC_HEADER_LENGTH;
      data_.reset(new io::OutputBuf(total_len));
      MessageHeader* reply_hdr;
      int size = RPC_HEADER_LENGTH;
      data_->Next((char**) &reply_hdr, &size);
      CHECK_EQ(size, RPC_HEADER_LENGTH);

      reply_hdr->fun_id = header.fun_id;
      SET_LAST_TAG(*reply_hdr);
      SET_RESPONSE_TAG(*reply_hdr);
      reply_hdr->length = msg_->ByteSize();
      reply_hdr->id = header.id;

      char* data;
      size = msg_->ByteSize();
      data_->Next(&data, &size);
      // TODO: ZeroCopyStream.
      msg_->SerializePartialToArray(data, size);

      iovec io;
      io.iov_base = reply_hdr;
      io.iov_len = total_len;
      iov_.push_back(io);
    }

    virtual ~ReplyObject() {
    }

  private:
    scoped_ptr<Message> msg_;

    std::vector<iovec> iov_;
    scoped_ptr<io::OutputBuf> data_;

    virtual const std::vector<iovec>& ioVec() const {
      return iov_;
    }

    DISALLOW_COPY_AND_ASSIGN(ReplyObject);
};

}
namespace rpc {

void RpcRequestHandler::process(io::Connection* conn, io::InputBuf* input_buf,
                                const TimeStamp& time_stamp) {
  const MessageHeader& header = GetRpcHeaderFromConnection(conn);
  MethodHandler * method_handler = handler_map_->FindMehodById(header.fun_id);
  if (method_handler == NULL) {
    LOG(WARNING)<< "can't find handler, id: " << header.fun_id;
    return;
  }

  DCHECK_EQ(input_buf->ByteCount(), RPC_HEADER_LENGTH);
  scoped_ptr<Message> req(method_handler->request->New());
  scoped_ptr<InputStream> input_stream(new InputStream(input_buf));
  bool ret = req->ParseFromZeroCopyStream(input_stream.get());
  if (!ret) {
    DLOG(WARNING)<< "parse request error: " << req->DebugString();
    return;
  }

  // reply will be released by Closure.
  Message* reply = method_handler->reply->New();
  method_handler->service->CallMethod(method_handler->method, NULL, req.get(),
                                      reply,
                                      new ReplyClosure(conn, header, reply));
}

RpcRequestHandler::ReplyClosure::ReplyClosure(io::Connection* conn,
                                              const MessageHeader& header,
                                              Message* reply)
    : hdr_(header), reply_(reply), conn_(conn) {
  conn->Ref();
}

RpcRequestHandler::ReplyClosure::~ReplyClosure() {
}

void RpcRequestHandler::ReplyClosure::Run() {
  io::OutputObject* obj = new io::OutVectorObject(
      new ReplyObject(hdr_, reply_.release()), true);
  conn_->Send(obj);

  delete this;
}

}
