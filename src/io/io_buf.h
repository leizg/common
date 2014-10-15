#ifndef IO_BUF_H_
#define IO_BUF_H_

#include "base/base.h"

namespace io {

class OutputObject {
 public:
  virtual ~OutputObject() {
  }

  // return true iif no data need to send.
  virtual bool Send(int fd, int32* err_no) = 0;

 protected:
  OutputObject()
      : offset_(0),
        left_(0) {
  }

  uint32 offset_;
  uint32 left_;

 private:
  DISALLOW_COPY_AND_ASSIGN(OutputObject);
};

class OutVectorObject : public OutputObject {
 public:
  class IoObject {
   public:
    virtual ~IoObject() {
    }

    virtual const std::vector<iovec>& IoVec() const = 0;
  };

  explicit OutVectorObject(IoObject* obj, bool self_delete = true)
      : obj_(obj),
        self_delete_(self_delete) {
    const std::vector<iovec>& data = obj->IoVec();
    for (uint32 i = 0; i < data.size(); ++i) {
      left_ += data[i].iov_len;
    }
  }
  virtual ~OutVectorObject() {
    if (self_delete_) delete obj_;
  }

 private:
  IoObject* obj_;
  bool self_delete_;

  void BuildData(std::vector<iovec>* io_vec) const;

  virtual bool Send(int fd, int32* err_no);

  DISALLOW_COPY_AND_ASSIGN(OutVectorObject);
};

}

#endif /* IO_BUF_H_ */
