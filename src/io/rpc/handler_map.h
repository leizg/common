#ifndef HANDLER_MAP_H_
#define HANDLER_MAP_H_

#include "rpc_def.h"
#include "base/base.h"

namespace rpc {
struct MethodHandler {
  Service* service;
  // shouldn't delete method.
  const MethodDescriptor* method;

  Message* request;
  Message* reply;
};

// FIXME: RwLock. ??? needed?
class HandlerMap {
 public:
  HandlerMap() {
  }
  virtual ~HandlerMap();

  void AddService(Service* serv);

  MethodHandler* FindMehodById(uint32 id) const {
    ServMap::const_iterator it = serv_map_.find(id);
    if (it == serv_map_.end()) return NULL;
    return it->second;
  }

 private:
  typedef std::map<uint32, MethodHandler*> ServMap;
  ServMap serv_map_;

  bool AddHandler(uint32 hash_id, MethodHandler* handler) {
    MethodHandler* old = FindMehodById(hash_id);
    if (old != NULL) return false;

    return serv_map_.insert(std::make_pair(hash_id, handler)).second;
  }

  DISALLOW_COPY_AND_ASSIGN(HandlerMap);
};
}

#endif /* HANDLER_MAP_H_ */
