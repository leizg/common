#ifndef LOGER_DEF_H_
#define LOGER_DEF_H_

namespace util {

enum RECORD_TYPE {
  START_RECORD = (1 << 0), LAST_RECORD = (1 << 1),
};

enum COPMRESS_TYPE {
  NO_COMPRESSION, SNAPPY_COMPRESSION = 1 << 0, GZIP_COMPRESSION = 1 << 1,
};

enum Status {
  OK = 0, FILE_,
};

#define BLOCK_SIZE (32U * 1024)
#define LOG_HEADER_SIZE 12

}

#endif /* LOGER_DEF_H_ */
