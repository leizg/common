#ifndef XX_HASH_H_
#define XX_HASH_H_

#include "data_types.h"
#include "xx_hash_internal.h"

inline uint32 XXHash32(const char* data, uint32 len, uint32 seed) {
  return static_cast<uint32>(XXH32(data, len, seed));
}
inline uint32 XXHash32(const char* data, uint32 len) {
  return XXHash32(data, len, 0);
}

inline uint32 XXHash32(const std::string& data, uint32 seed) {
  return XXHash32(data.data(), static_cast<uint32>(data.size()), seed);
}
inline uint32 XXHash32(const std::string& data) {
  return XXHash32(data, 0);
}

class XXHash {
  public:
    explicit XXHash(uint32 seed = 0) {
      stat_ = XXH32_init(seed);
    }

    bool update(const char* data, uint32 len) {
      auto ret = XXH32_update(stat_, data, len);
      return XXH_OK == ret;
    }
    bool update(const std::string& data) {
      return update(data.data(), static_cast<uint32>(data.size()));
    }

    uint32 digest() {
      return static_cast<uint32>(XXH32_digest(stat_));
    }

  private:
    void* stat_;

    // disallow copy and assign.
    XXHash(const XXHash&);
    void operator=(const XXHash&);
};

#endif /* XX_HASH_H_ */
