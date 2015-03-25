#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <cstdlib>

inline void SetNext(void* item, void* next) {
  *(reinterpret_cast<void**>(item)) = next;
}

inline void* GetNext(void* item) {
  return *(reinterpret_cast<void**>(item));
}

inline void ListPush(void** list, void* item) {
  SetNext(item, *list);
  *list = item;
}

inline void* ListPop(void** list) {
  void* item = *list;
  *list = GetNext(item);
  return item;
}

inline void ListPushRange(void** head, void* start, void* end) {
  if (start != nullptr) {
    SetNext(end, *head);
    *head = start;
  }
}

inline int ListPopRange(void** head, int n, void** start, void** end) {
  *start = nullptr;
  *end = nullptr;

  int size;
  void* item = nullptr;
  for (size = 0; size < n; ++size) {
    void* item = ListPop(head);
    if (size == 0) {
      *start = item;
    }
  }

  if (size != 0) {
    *end = item;
  }

  return size;
}

inline int ListSize(void** list) {
  int size;

  void *item = *list;
  for (size = 0; item != nullptr; size++) {
    item = GetNext(item);
  }
  return size;
}

#endif /* LINKED_LIST_H_ */
