#ifndef ZERO_COPY_STREAM_H_
#define ZERO_COPY_STREAM_H_

#include "base/base.h"

class ZeroCopyInputStream {
 public:
  virtual ~ZeroCopyInputStream() {
  }

  // return 0 iif no data.
  virtual int32 Next(char** buf, uint32* len) = 0;

  virtual char* Skip(uint32 len) = 0;
  virtual void Backup(uint32 len) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(ZeroCopyInputStream);
};

class ZeroCopyOutputStream {
 public:
  virtual ~ZeroCopyOutputStream() {
  }

  virtual void Next(char** buf, uint32* len) = 0;

  virtual void Backup(uint32 len) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(ZeroCopyOutputStream);
};

#endif /* ZERO_COPY_STREAM_H_ */
