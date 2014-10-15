#include "handler_map.h"
#include "rpc_processor.h"

#include "io/io_buf.h"
#include "io/input_buf.h"
#include "io/output_buf.h"
#include "io/connection.h"

namespace {

class ReplyObject : public io::OutVectorObject::IoObject {
 public:
  ReplyObject(const MessageHeader& header, Message* reply)
      : msg_(reply),
        offset_(0) {
    data_.reset(new io::OutputBuf(msg_->ByteSize() + RPC_HEADER_LENGTH));
    MessageHeader* reply_hdr;
    uint size = RPC_HEADER_LENGTH;
    data_->Next((char**) &reply_hdr, &size);
    CHECK_EQ(size, RPC_HEADER_LENGTH);

    reply_hdr->fun_id = header.fun_id;
    reply_hdr->rpc_type = RPC_RESPONSE;
    reply_hdr->length = msg_->ByteSize();
    reply_hdr->id = header.id;

    char* data;
    size = msg_->ByteSize();
    data_->Next(&data, &size);
    // FIXME: ZeroCopyStream.
    msg_->SerializePartialToArray(data, size);

    iovec io;
    io.iov_base = data_->peekR();
    io.iov_len = data_->size();
    iov_.push_back(io);
  }

  virtual ~ReplyObject() {
  }

 private:
  scoped_ptr<Message> msg_;

  uint32 offset_;
  std::vector<iovec> iov_;
  scoped_ptr<io::OutputBuf> data_;

  virtual const std::vector<iovec>& IoVec() const {
    return iov_;
  }

  DISALLOW_COPY_AND_ASSIGN(ReplyObject);
};

class ReplyClosure : public ::google::protobuf::Closure {
 public:
  ReplyClosure(io::Connection* conn, const MessageHeader& header, Message* req,
               Message* reply)
      : hdr_(header),
        req_(req),
        reply_(reply),
        conn_(conn) {
    conn->Ref();
  }
  virtual ~ReplyClosure() {
  }

 private:
  const MessageHeader hdr_;
  scoped_ptr<Message> req_;
  scoped_ptr<Message> reply_;

  scoped_ref<io::Connection> conn_;

  virtual void Run() {
    io::OutputObject* obj = new io::OutVectorObject(
        new ReplyObject(hdr_, reply_.release()), true);
    conn_->Send(obj);

    delete this;
  }

  DISALLOW_COPY_AND_ASSIGN(ReplyClosure);
};

}

namespace rpc {

void RpcProcessor::Dispatch(io::Connection* conn, io::InputBuf* input_buf,
                            const TimeStamp& time_stamp) {
  const MessageHeader& header = GetRpcHeaderFromConnection(conn);
  MethodHandler * method_handler = handler_map_->FindMehodById(header.fun_id);
  if (method_handler == NULL) {
    LOG(WARNING)<< "can't find handler, id: " << header.fun_id;
    return;
  }

  // req and reply are released by Closure.
  Message* req = method_handler->request->New();
  // TODO: zeroCopyStream.
  bool ret = req->ParseFromArray(input_buf->peekR(), input_buf->size());
  if (!ret) {
    DLOG(WARNING)<< "parse data error: " << req->DebugString();
    delete req;
  }

  Message* reply = method_handler->reply->New();
  method_handler->service->CallMethod(
      method_handler->method, NULL, req, reply,
      new ReplyClosure(conn, header, req, reply));
}

}
