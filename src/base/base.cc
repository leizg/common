#include "base/base.h"

void SplitString(const std::string& src, char c,
                 std::vector<std::string>* vec) {
  vec->clear();
  if (!src.empty()) {
    uint32 pos = 0;

    while (true) {
      std::string::size_type ret = src.find(c, pos);
      std::string sub = src.substr(pos, ret);
      if (pos != ret && !sub.empty()) {
        vec->push_back(sub);
        if (ret == std::string::npos) {
          break;
        }
      }
      pos++;
    }
  }
}

