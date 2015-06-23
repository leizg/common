#ifndef HASH_H_
#define HASH_H_

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "base/data_types.h"

// From http://www.azillionmonkeys.com/qed/hash.html
// This is the hash used on WebCore/platform/stringhash
uint32 SuperFastHash(const char * data, int len);
inline uint32 SuperFastHash(const char* key, size_t length) {
  return SuperFastHash(key, static_cast<int>(length));
}

inline uint32 SuperFastHash(uint32 id) {
  return id;
}
inline uint64 SuperFastHash(uint64 id) {
  return id;
}

inline uint32 SuperFastHash(const std::string& key) {
  if (key.empty()) return 0;
  return SuperFastHash(key.data(), key.size());
}

#endif /* HASH_H_ */
