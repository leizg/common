#pragma once

#define INVALID_TID (-1)
#define INVALID_FD (-1)

#define DISALLOW_COPY_AND_ASSIGN(TypeName)    \
  TypeName(const TypeName&);                  \
  void operator=(const TypeName&)

#define EXPECT_TRUE(expr) (__builtin_expect(!!(expr), 1))
#define EXPECT_FALSE(expr) (__builtin_expect(!(expr), 1))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#define ALIGN_SIZE (4)
#define alignSize(v, a) \
  (((v) + (a) - 1) & (~((a) -1)))
#define ALIGN(val) alignSize(val, ALIGN_SIZE)

#define closeWrapper(fd) \
  do { \
    if (fd == INVALID_FD) break; \
    while (true) { \
      int __ret = ::close(fd); \
      if (__ret != -1) break ; \
      switch(errno) { \
        case EINTR: \
          continue; \
        default: \
          PLOG(ERROR) << "close error"; \
      } \
    } \
    fd = INVALID_FD; \
  } while(0)

