#include "handler_map.h"

namespace rpc {

HandlerMap::~HandlerMap() {
  for (ServMap::const_iterator it = serv_map_.begin(); it != serv_map_.end();
      ++it) {
    const MethodHandler* const h = it->second;
    delete h->request;
    delete h->reply;
    delete h;
  }
  serv_map_.clear();
}

void HandlerMap::AddService(Service* serv) {
  const google::protobuf::ServiceDescriptor* const serv_desc =
      serv->GetDescriptor();

  int method_count = serv_desc->method_count();
  for (int i = 0; i < method_count; ++i) {
    const MethodDescriptor* const method_desc = serv_desc->method(i);

    MethodHandler* handler = new MethodHandler;
    handler->service = serv;
    handler->method = method_desc;
    handler->request = serv->GetRequestPrototype(method_desc).New();
    handler->reply = serv->GetResponsePrototype(method_desc).New();

    const std::string& method_name = method_desc->full_name();
    uint32 hash_id = Hash(method_name);
    if (!AddHandler(hash_id, handler)) {
      delete handler->request;
      delete handler->reply;
      delete handler;
      LOG(WARNING)<<"add handler error, name: " << method_name;
    }
  }
}

}
