#include "echo_client.h"
#include "async/test/echo_coder.h"
#include "async/test/echo_protocol.h"
#include "echo_client_responser.h"

#include "io/io_buf.h"
#include "io/memory_block.h"
#include "async/connector.h"
#include "async/tcp_client.h"
#include "async/event_manager.h"

namespace {
class EchoObject : public io::OutVectorObject::IoObject {
  public:
    EchoObject() {
      buildData();
    }
    virtual ~EchoObject() {
    }

  private:
    std::vector<iovec> data_;
    scoped_ptr<io::ExternableChunk> buf_;

    void buildData();
    virtual const std::vector<iovec>& ioVec() const {
      return data_;
    }

    static uint64 value_;

    DISALLOW_COPY_AND_ASSIGN(EchoObject);
};

uint64 EchoObject::value_ = 1;

void EchoObject::buildData() {
  buf_.reset(new io::ExternableChunk(64));
  uint64 val = value_++;
  iovec io;
  const char* begin = buf_->peekR();
  CHECK(test::Encode((const char* )&val, sizeof(val), buf_.get()));
  char* end = (char*)buf_->peekR();
  io.iov_base = (char*) begin;
  io.iov_len = end - begin;
  DCHECK_EQ(io.iov_len, 4 + 8);
  data_.push_back(io);
}
}

namespace test {

EchoClient::EchoClient(async::EventManager* ev_mgr, uint32 count)
    : test_number_(count), ev_mgr_(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);
  protocol_.reset(new EchoProtocol(new EchoResponser));
}

EchoClient::~EchoClient() {
}

bool EchoClient::connect(const std::string& ip, uint16 port) {
  client_.reset(async::TcpClient::create(ev_mgr_, ip, port));
  client_->setProtocol(protocol_.get());
  if (!client_->connect(3)) {
    LOG(WARNING)<< "connect error: " << ip << ": " << port;
    return false;
  }

  return true;
}

void EchoClient::startTest() {
  for (uint32 i = 0; i < test_number_; ++i) {
    io::OutputObject* obj = new io::OutVectorObject(new EchoObject);
    client_->send(obj);
  }
}

void EchoClient::waitForFinished() {
}

}
