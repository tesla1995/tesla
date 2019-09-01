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
// Date: Tue Jul  9 07:29:14 CST 2019

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <memory>
#include <atomic>

namespace tesla {
namespace wait_free {

template <typename T>
class LockFreeStackWithReferenceCount {
 public:
  LockFreeStackWithReferenceCount() {
    //if (false == std::atomic_is_lock_free(&head_)) {
    //  fprintf(stderr, "operations on `head_' are not lock-free!\n");
    //  abort();
    //}
  }

  ~LockFreeStackWithReferenceCount() {
    uint64_t count = 0;
    while (Pop()) { count++; }
    fprintf(stdout, "%lu nodes in the list when destructing!\n", count);
  }
 public:
  struct Node;

  struct CountedNodePtr {
    int64_t external_count{0};
    Node* ptr{nullptr};
  };

  struct Node {
    std::shared_ptr<T> data;
    std::atomic<int64_t> internal_count{0};
    CountedNodePtr next;

    Node(const T& data) : data(std::make_shared<T>(data)) {}
  };

  void Push(const T& data) {
    CountedNodePtr new_node;
    Node* node = new Node(data);
    new_node.ptr = node;
    new_node.external_count = 1;

    new_node.ptr->next = head_.load();
    while (!head_.compare_exchange_weak(new_node.ptr->next, new_node));
  }

  std::shared_ptr<T> Pop() {
    CountedNodePtr old_head = head_.load();
    while (1) {
      IncreaseHeadExternalCount(old_head);
      // we can safely dereference pointer in `old_head' later.
      
      Node* ptr = old_head.ptr;
      // no nodes in the list.
      if (nullptr == ptr) {
        return std::shared_ptr<T>();
      }

      // FIXME: add annotation.
      if (head_.compare_exchange_strong(old_head, ptr->next)) {
        std::shared_ptr<T> res;
        res.swap(ptr->data);
        int64_t external_count = old_head.external_count - 2;
        if (ptr->internal_count.fetch_add(external_count) == -external_count) {
          delete ptr;
        }
        return res;
      } else if (ptr->internal_count.fetch_sub(1) == 1) {
        // There's some problem here.
        delete ptr;
      }
    }
  }

 private:
  void IncreaseHeadExternalCount(CountedNodePtr& old_head) {
    CountedNodePtr new_head;
    do {
      new_head = old_head;
      ++new_head.external_count;
    } while (!head_.compare_exchange_strong(old_head, new_head));
    old_head.external_count = new_head.external_count;
  }

  std::atomic<CountedNodePtr> head_;
};

}  // wait_free
}  // tesla
