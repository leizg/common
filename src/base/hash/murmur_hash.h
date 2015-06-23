#pragma once

#include <cstdint>
#include <string>

uint32_t murmur_hash32(const char* data, int len, uint32_t seed);
inline uint32_t murmur_hash32(const char* data, int len) {
  return murmur_hash32(data, len, 0);
}
inline uint32_t murmur_hash32(const std::string& s, uint32_t seed) {
  return murmur_hash32(s.data(), static_cast<int>(s.size()), seed);
}
inline uint32_t murmur_hash32(const std::string& s) {
  return murmur_hash32(s, 0);
}

uint64_t murmur_hash64(const char* data, int len, uint64_t seed);
inline uint64_t murmur_hash64(const char* data, int len) {
  return murmur_hash64(data, len, 0);
}
inline uint64_t murmur_hash64(const std::string& s, uint64_t seed) {
  return murmur_hash64(s.data(), static_cast<int>(s.size()), seed);
}
inline uint64_t murmur_hash64(const std::string& s) {
  return murmur_hash64(s, 0);
}

