#pragma once

#include "base/base.h"

namespace util {

inline const std::string int2Str(uint64 i) {
  std::string val;
  if (i == 0) val = "0";
  else {
    while (i != 0) {
      auto j = i % 10;
      val.insert(0, 1, j + '0');
      i = i / 10;
    }
  }
  return val;
}

inline int64 str2Int(const std::string& str) {
  if (str.empty()) return 0;
  int64 val = 0;
  auto pos = 0;
  while (pos < str.size()) {
    if (str[pos] != '0') {
      break;
    }
  }

  for (auto i = str.size() - 1; i >= pos; --i) {
    val *= 10;
    val += str[i] - '0';
  }

  return val;
}
}
