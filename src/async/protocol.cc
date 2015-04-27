#include "protocol.h"
#include "connection.h"
#include "event_manager.h"
#include "io/memory_block.h"
#include "io/input_stream.h"

namespace {

io::InputStream* releaseStream(async::ProReactorProtocol::UserData* ud) {
  ud->newPackage();

  io::InputStream* stream(new io::InputStream(ud->src.release()));
  ud->src.reset(new io::ConcatenaterSource);

  return stream;
}
}

namespace async {

ProReactorProtocol::UserData::UserData()
    : is_last(true), io_stat(IO_START), pending_size(0) {
  chunk.reset(new io::ExternableChunk);
  src.reset(new io::ConcatenaterSource);
  out_queue.reset(new io::OutQueue);
}

void ProReactorProtocol::UserData::newPackage() {
  src->push(new io::ChunkSource(chunk.release()));
  chunk.reset(new io::ExternableChunk);
}

const char* ProReactorProtocol::UserData::peekHeader() const {
  return chunk->peekR();
}

ProReactorProtocol::UserData::~UserData() {
}

void ProReactorProtocol::handleRead(Connection* conn, TimeStamp time_stamp) {
  UserData* u = reinterpret_cast<UserData*>(conn->getData());
  if (!RecvPending(conn, u)) return;

  while (true) {
    switch (u->io_stat) {
      case IO_START:
        u->io_stat = IO_HEADER;
        if (!recvData(conn, u, parser_->headerLength())) {
          return;
        }

      case IO_HEADER:
        u->io_stat = IO_BODY;
        if (!parser_->parseHeader(conn)) {
          if (reporter_ != nullptr) {
            reporter_->report(conn);
          }
          return;
        }

      case IO_BODY:
        u->io_stat = IO_END;
        if (!recvData(conn, u, u->pending_size)) {
          return;
        }

      case IO_END:
        if (u->is_last) {
          scheluder_->dispatch(conn, releaseStream(u), time_stamp);
          u->is_last = false;
          u->io_stat = IO_START;
          return;
        }

        u->newPackage();
        u->io_stat = IO_START;
        break;
    }
  }
}

bool ProReactorProtocol::handleWrite(Connection* conn, TimeStamp time_stamp,
                                     int* err_no) {
  DCHECK_NOTNULL(err_no);
  UserData* ud = reinterpret_cast<UserData*>(conn->getData());
  if (!ud->out_queue->empty()) {
    if (!ud->out_queue->send(conn->fileHandle(), err_no)) {
      if (*err_no != EWOULDBLOCK) {
        handleClose(conn);
      }
      return false;
    }
  }

  return true;
}

void ProReactorProtocol::handleError(Connection* conn) {
  handleClose(conn);
}

void ProReactorProtocol::handleClose(Connection* conn) {
  if (reporter_ != nullptr) {
    reporter_->report(conn);
  }

  conn->handleClose();
}

bool ProReactorProtocol::recvData(Connection* conn, UserData* u,
                                  uint32 data_len) {
  u->pending_size = data_len;
  return RecvPending(conn, u);
}

bool ProReactorProtocol::RecvPending(Connection* conn, UserData* ud) {
  if (ud->pending_size == 0) return true;

  int size = ud->pending_size;
  ud->chunk->ensureLeft(size);
  if (!conn->read(ud->chunk->peekW(), &size)) {
    handleError(conn);
    return false;
  }

  DCHECK_GT(size, 0);
  DCHECK_GE(ud->pending_size, size);
  ud->pending_size -= size;
  ud->chunk->skipWrite(size);

  return 0 == ud->pending_size;
}

}
