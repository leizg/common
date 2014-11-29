#ifndef MACRO_DEF_H_
#define MACRO_DEF_H_

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

#endif
