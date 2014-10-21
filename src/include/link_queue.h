#ifndef LINK_QUEUE_H_
#define LINK_QUEUE_H_

#include "base/base.h"

struct LinkNode {
    virtual ~LinkNode() {
    }

    LinkNode* prev;
    LinkNode* next;

    void Remove() {
      if (prev != next) {
        prev->next = next;
        next->prev = prev;
        prev = next = this;
      }
    }
    void InertAfter(LinkNode* node) {
      node->prev = this;
      node->next = next;
      next->prev = node;
      next = node;
    }
    void InertBefore(LinkNode* node) {
      node->next = this;
      node->prev = prev;
      prev->next = node;
      prev = node;
    }

  protected:
    LinkNode()
        : prev(this), next(this) {
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(LinkNode);
};

class LinkQueue : public LinkNode {
  public:
    virtual ~LinkQueue() {
      CHECK(empty());
    }

    bool empty() const {
      return prev == next;
    }

    void clear() {
      while (!empty()) {
        LinkNode* node = next;
        node->Remove();
        delete node;
      }
    }

  protected:
    LinkQueue()
        : prev(this), next(this) {
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(LinkQueue);
};

#endif /* LINK_QUEUE_H_ */
