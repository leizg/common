#include "epoller.h"

namespace io {

Epoller::~Epoller() {
  if (ep_fd_ != INVALID_FD) {
    ::close(ep_fd_);
  }
}

bool Epoller::Init() {
}

bool Epoller::Loop() {
}

bool Epoller::LoopInAnotherThread() {
}

void Epoller::Stop() {
}

bool Epoller::Add(Event* ev) {
}

void Epoller::Mod(Event* ev) {
}

void Epoller::Del(const Event& ev) {
}

}
