#ifndef LINK_QUEUE_H_
#define LINK_QUEUE_H_

#include "base/base.h"

struct LinkNode {
    virtual ~LinkNode() {
    }

    LinkNode* prev;
    LinkNode* next;

    void remove() {
      if (prev != next) {
        prev->next = next;
        next->prev = prev;
        prev = next = this;
      }
    }
    void inertAfter(LinkNode* node) {
      node->prev = this;
      node->next = next;
      next->prev = node;
      next = node;
    }
    void inertBefore(LinkNode* node) {
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
    }

    bool empty() const {
      return prev == next;
    }

  protected:
    LinkQueue()
        : prev(this), next(this), size_(0) {
    }

    uint64 size_;

  private:
    DISALLOW_COPY_AND_ASSIGN(LinkQueue);
};

#endif /* LINK_QUEUE_H_ */
