// Copyright (c) 2019 Tesla, Inc.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael Tesla (michaeltesla1995@gmail.com)
// Date: Sat Jul  6 17:31:56 CST 2019

#include "wait_free/hazard_version.h"
#include <string.h>

namespace tesla {
namespace wait_free {

template <typename T>
class LockFreeStack;

template <typename T>
class LifoNode : public hazard_version::HazardNode {
  friend class LockFreeStack<T>;
 public:
  LifoNode() : next_(nullptr) {}
  LifoNode(const T& value) : next_(nullptr), value_(value) {}
  ~LifoNode() override {}
 public:
  void Retire() override { value_ = T(); next_ = nullptr; }
 public:
  void set_next(LifoNode* next) { next_ = next; }
  LifoNode* next() { return next_; }
  LifoNode*& mutable_next() { return next_; }
 public:
  T value() { return value_; }
 private:
  LifoNode* next_;
  T value_;
};

template <typename T>
class LockFreeStack {
  using Node = LifoNode<T>;
 public:
  LockFreeStack() : head_(nullptr) {}
  ~LockFreeStack() {
    Node* top = head_.load();
    int64_t count = 0;
    while (top) {
      // Note: for exception safety, don't call Retire() when modifying list.
      Node* current = top;
      top = top->next();
      current->Retire();
      ++count;
    }
    fprintf(stdout, "~LockFreeStack:: remaining count[%ld]\n", count);
  }
 public:
  void Push(Node* node) {
    // FIXME: exception safety?
    // node = new(node) Node(value);

    node->set_next(head_.load());
    while (!head_.compare_exchange_weak(node->mutable_next(), node));
  }

  bool Pop(T &value) {
    bool ret = false;
    
    uint64_t handle = 0;
    hazard_version_.Acquire(handle);

    // ONLY one thread could extract the node that `head_' point to from the list.
    Node* old_head = head_.load();
    while (old_head && !head_.compare_exchange_weak(old_head, old_head->next()));

    if (old_head) {
      value = old_head->value();
      hazard_version_.AddNode(old_head);
      ret = true;
    }

    hazard_version_.Release(handle);
    return ret;
  }
 private:
  hazard_version::HazardVersion hazard_version_;
  HAZARD_CACHELINE_ALIGNMENT std::atomic<Node*> head_;
};

}  // wait_free
}  // tesla
