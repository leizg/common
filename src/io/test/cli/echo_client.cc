#include "echo_client.h"
#include "io/test/echo_coder.h"
#include "io/test/echo_protocol.h"
#include "echo_client_responser.h"

#include "io/io_buf.h"
#include "io/output_buf.h"
#include "io/connector.h"
#include "io/tcp_client.h"
#include "io/event_manager.h"

namespace {
class EchoObject : public io::OutVectorObject::IoObject {
  public:
    EchoObject() {
      buf_.reset(new io::OutputBuf(64));
      buildData();
    }
    virtual ~EchoObject() {
    }

  private:
    std::vector<iovec> data_;
    scoped_ptr<io::OutputBuf> buf_;

    void buildData();
    virtual const std::vector<iovec>& ioVec() const {
      return data_;
    }

    static uint64 value_;

    DISALLOW_COPY_AND_ASSIGN(EchoObject);
};

uint64 EchoObject::value_ = 1;

void EchoObject::buildData() {
  uint64 val = value_++;
  iovec io;
  char* begin = buf_->peek();
  CHECK(test::Encode((const char* )&val, sizeof(val), buf_.get()));
  char* end = buf_->peek();
  io.iov_base = begin;
  io.iov_len = end - begin;
  data_.push_back(io);
}
}

namespace test {

EchoClient::EchoClient(io::EventManager* ev_mgr, uint32 count)
    : test_number_(count), ev_mgr_(ev_mgr) {
  DCHECK_NOTNULL(ev_mgr);
  protocol_.reset(new EchoProtocol(new EchoResponser));
}

EchoClient::~EchoClient() {
}

bool EchoClient::connect(const std::string& ip, uint16 port) {
  client_.reset(new io::TcpClient(ev_mgr_, ip, port));
  client_->SetProtocol(protocol_.get());
  if (!client_->Connect(3)) {
    LOG(WARNING)<< "connect error: " << ip << ": " << port;
    return false;
  }

  return true;
}

void EchoClient::startTest() {
  for (uint32 i = 0; i < test_number_; ++i) {
    io::OutputObject* obj = new io::OutVectorObject(new EchoObject);
    client_->Send(obj);
  }
}

void EchoClient::waitForFinished() {
}

}
