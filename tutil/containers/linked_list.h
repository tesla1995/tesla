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
// Date: Sat Mar  2 12:23:41 CST 2019

#include "tutil/macros.h"

#ifndef TESLA_TUTIL_CONTAINERS_LINKED_LIST_H_
#define TESLA_TUTIL_CONTAINERS_LINKED_LIST_H_

namespace tesla {
namespace tutil {

template <typename T>
class LinkNode {
 public:
  // LinkNode is self-referential as default.
  LinkNode() : previous_(this), next_(this) {}

  LinkNode(LinkNode<T>* previous, LinkNode<T>* next)
      : previous_(previous), next_(next) {}

  DISALLOW_COPY_AND_ASSIGN(LinkNode);

  // Insert |this| into the linked list, before |e|.
  void InsertBefore(LinkNode<T>* e) {
    this->previous_ = e->previous_;  
    this->next_ = e;
    e->previous_->next_ = this;
    e->previous_ = this;
  }

  // Insert |this| as a circular linked list into the linked list, before |e|.
  void InsertBeforeAsList(LinkNode<T>* e) {
    LinkNode<T>* prev = this->previous_;
    prev->next_ = e; 
    this->previous_ = e->previous_;
    e->previous_->next_ = this;
    e->previous_ = prev;
  }

  // Insert |this| into the linked list, after |e|.
  void InsertAfter(LinkNode<T>* e) {
    this->previous_ = e;
    this->next_ = e->next_;
    e->next_->previous_ = this;
    e->next_ = this;
  }

  // Insert |this| as a circular linked list into the linked list, after |e|.
  void InsertAfterAsList(LinkNode<T>* e) {
    LinkNode<T>* prev = this->previous_;
    previous_->next_ = e->next_;
    this->previous_ = e;
    e->next_->previous_ = prev;
    e->next_ = this;
  }

  void RemoveFromList() {
    this->previous_->next_ = this->next_; 
    this->next_->previous_ = this->previous_;
    this->previous_ = this;
    this->next_ = this;
  }

  LinkNode<T>* previous() const {
    return previous_;
  }

  LinkNode<T>* next() const {
    return next_; 
  }

  // Cast from the node-type to the value-type.
  const T* value() const {
    return static_cast<const T*>(this);
  }

  T* value() {
    return static_cast<T*>(this);
  }


 private:
  LinkNode<T>* previous_{nullptr};
  LinkNode<T>* next_{nullptr};
};

template <typename T>
class LinkedList {
 public:
  // The "root" node is self-referential, and forms the basis of a circular
  // list.(root.next() will point back to the start of the list and 
  // root.previous() wraps around to the end of the list).
  LinkedList() {}

  DISALLOW_COPY_AND_ASSIGN(LinkedList);

  // Appends |e| to the end of the linked list.
  void Append(LinkNode<T>* e) {
    e->InsertBefore(&root_);
  }

  LinkNode<T>* head() const {
    return root_.next();
  }

  LinkNode<T>* tail() const {
    return root_.previous();
  }

  const LinkNode<T>* end() const {
    return &root_;
  }

  bool empty() const {
    return head() == end();
  }

 private:
  LinkNode<T> root_;
};

} // tutil
} // tesla

#endif // TESLA_TUTIL_CONTAINERS_LINKED_LIST_H_
