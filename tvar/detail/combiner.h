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
// Date: Sat Apr  6 21:45:13 CST 2019

#ifndef TESLA_TVAR_DETAIL_COMBINER_H_
#define TESLA_TVAR_DETAIL_COMBINER_H_

#include <atomic>       // std::atomic
#include <mutex>        // std::mutex std::lock_guard
#include <type_traits>  // std::enable_if

#include "log/logging.h"
#include "tutil/containers/linked_list.h"
#include "tutil/type_traits.h"
#include "tvar/detail/agent_group.h"
#include "tvar/detail/call_op_returning_void.h"
#include "tvar/detail/is_atomical.h"

namespace tesla {
namespace tvar {
namespace detail {

// Parameter to merge_global.
template <typename Combiner>
class GlobalValue {
 public:
  using result_type = typename Combiner::result_type;
  using agent_type = typename Combiner::Agent;

  GlobalValue(agent_type* agent, Combiner* combiner)
      : agent_(agent), combiner_(combiner) {}

  ~GlobalValue() {}

  // Call this method to unlock tls element and lock the combiner.
  // Unlocking tls element avoids potential deadlock with
  // AgentCombiner::reset(), which also means that tls element may be
  // changed during calling of this method. BE AWARE OF THIS!
  // After this method is called (and before unlock), tls element and
  // global_result will not be changed provided this method is called
  // from the thread owning the agent.
  result_type* lock() {
    agent_->element.lock_.unlock();
    combiner_->lock_.lock();
    return &combiner_->global_result_;
  }

  // Call this method to unlock the combiner and lock tls element again.
  void unlock() {
    combiner_->lock_.unlock();
    agent_->element.lock_.lock();
  }

 private:
  agent_type* agent_{nullptr};
  Combiner* combiner_{nullptr};
};

// Abstraction of tls element whose operations are all atomic.
template <typename T, typename Enable = void>
class ElementContainer {
  template <typename>
  friend class GlobalValue;

 public:
  void load(T* out) {
    std::lock_guard<std::mutex> guard(lock_);
    *out = value_;
  }

  void store(const T& new_value) {
    std::lock_guard<std::mutex> guard(lock_);
    value_ = new_value;
  }

  void exchange(T* old_value, const T& new_value) {
    std::lock_guard<std::mutex> guard(lock_);
    *old_value = value_;
    value_ = new_value;
  }

  template <typename Op, typename T2>
  void modify(const Op& op, const T2& v2) {
    std::lock_guard<std::mutex> guard(lock_);
    call_op_returning_void(op, value_, v2);
  }

  template <typename Op, typename GlobalValue>
  void merge_global(const Op& op, GlobalValue& global_value) {
    lock_.lock();
    op(global_value, value_);
    lock_.unlock();
  }

 private:
  T value_;
  std::mutex lock_;
};

// For simple variables, atomic variables is faster than lock.
template <typename T>
class ElementContainer<T,
                       typename std::enable_if<is_atomical<T>::value>::type> {
 public:
  // We don't need any memory fencing here, every op is relaxed.

  void load(T* out) { *out = value_.load(std::memory_order_relaxed); }

  void store(T new_value) {
    value_.store(new_value, std::memory_order_relaxed);
  }

  void exchange(T* old_value, T new_value) {
    *old_value = value_.exchage(new_value, std::memory_order_relaxed);
  }

  // Note: we must ensure that modify is a atomic operation.
  template <typename Op, typename T2>
  void modify(const Op& op, const T2& v2) {
    T old_value;
    T new_value;
    // There's a contention with the reset operation of combiner,
    // if the tls valus has been modified during op_, the
    // compare_exchange_weak operation will fail and recalculation
    // is to be processed according to the new version of value.
    do {
      old_value = value_.load(std::memory_order_relaxed);
      new_value = old_value;
      call_op_returning_void(op, new_value, v2);
    } while (!value_.compare_exchange_weak(old_value, new_value,
                                           std::memory_order_relaxed));
  }

  // [Unique]
  inline bool compare_exchange_weak(T& expected, T new_value) {
    return value_.compare_exchange_weak(expected, new_value,
                                        std::memory_order_relaxed);
  }

 private:
  std::atomic<T> value_;
};

template <typename ResultTp, typename ElementTp, typename BinaryOp>
class AgentCombiner {
 public:
  using result_type = ResultTp;
  using element_type = ElementTp;
  using self_type = AgentCombiner<ResultTp, ElementTp, BinaryOp>;

  friend class GlobalValue<self_type>;

 public:
  struct Agent : public tutil::LinkNode<Agent> {
    Agent() = default;

    ~Agent() {
      if (combiner) {
        combiner->CommitAndErase(this);
        combiner = nullptr;
      }
    }

    void reset(const ElementTp& val, self_type* c) {
      combiner = c;
      element.store(val);
    }

    // NOTE: Only available to non-atomic types.
    template <typename Op>
    void merge_global(const Op& op) {
      GlobalValue<self_type> g(this, combiner);
      element.merge_global(op, g);
    }

    self_type* combiner{nullptr};
    ElementContainer<ElementTp> element;
  };  // struct Agent

  using AgentGroup = detail::AgentGroup<Agent>;

  explicit AgentCombiner(const ResultTp result_identity = ResultTp(),
                         const ElementTp element_identity = ElementTp(),
                         const BinaryOp& op = BinaryOp())
      : id_(AgentGroup::CreateNewAgent()),
        op_(op),
        global_result_(result_identity),
        result_identity_(result_identity),
        element_identity_(element_identity) {}

  ~AgentCombiner() {
    if (id_ >= 0) {
      ClearAllAgents();
      // Don't forget to call it. We need to reuse id_.
      AgentGroup::DestroyAgent(id_);
      id_ = -1;
    }
  }

  const BinaryOp& op() const { return op_; }

  bool Valid() const { return id_ >= 0; }

  typename tutil::add_cr_non_integral<ElementTp>::type element_identity()
      const {
    return element_identity_;
  }

  typename tutil::add_cr_non_integral<ResultTp>::type result_identity() const {
    return result_identity_;
  }

  // Gets or creates agent by `id_' for threads that add values
  // to reducer. We need this function to be as fast as possible.
  inline Agent* GetOrCreateTlsAgent() {
    Agent* agent = AgentGroup::GetTlsAgent(id_);
    if (!agent) {
      // Create the agent
      agent = AgentGroup::GetOrCreateTlsAgent(id_);
      if (nullptr == agent) {
        LOG_ERROR << "Fail to create agent";
        return nullptr;
      }
    }
    // First call to this function, agent->combiner must be nullptr
    // no matter id_ is new or reused.
    if (agent->combiner) {
      return agent;
    }
    agent->reset(element_identity_, this);
    {
      std::lock_guard<std::mutex> guard(lock_);
      agents_.Append(agent);
    }
    return agent;
  }

  // Always called from the thread owning the agent.
  void CommitAndErase(Agent* agent) {
    if (nullptr == agent) {
      return;
    }

    ElementTp local;
    std::lock_guard<std::mutex> guard(lock_);
    agent->element.load(&local);
    call_op_returning_void(op_, global_result_, local);
    agent->RemoveFromList();
    // Here, we don't need to call reset() of agent because
    // agent is destructing.
  }

  // Always called from the thread owning the agent.
  void CommitAndClear(Agent* agent) {
    if (nullptr == agent) {
      return;
    }

    ElementTp prev;
    std::lock_guard<std::mutex> guard(lock_);
    agent->element.exchage(&prev, element_identity_);
    call_op_returning_void(op_, global_result_, prev);
  }

  // [Threadsafe] May be called from anywhere.
  ResultTp CombineAllAgents() const {
    ElementTp tls_value;
    std::lock_guard<std::mutex> guard(lock_);
    // If there's a thread exiting before destructing AgentCombiner, it's
    // Agent will be added to `global_result_' by call_op_returning_void.
    ResultTp ret = global_result_;
    for (tutil::LinkNode<Agent>* node = agents_.head(); node != agents_.end();
         node = node->next()) {
      node->value()->element.load(&tls_value);
      call_op_returning_void(op_, ret, tls_value);
    }
    return ret;
  }

  // [Threadsafe] May be called from anywhere.
  ResultTp ResetAllAgents() const {
    ElementTp prev;
    std::lock_guard<std::mutex> guard(lock_);
    ResultTp ret = global_result_;
    global_result_ = result_identity_;
    for (tutil::LinkNode<Agent>* node = agents_.head(); node != agents_.end();
         node = node->next()) {
      node->value()->element.exchange(&prev, element_identity_);
      call_op_returning_void(op_, ret, prev);
    }
    return ret;
  }

  void ClearAllAgents() {
    std::lock_guard<std::mutex> guard(lock_);
    // Reseting agents is necessary because the agent object may be reused.
    // Set element to be default-constructed so that if it's non-pod,
    // internal allocations should be released.
    for (tutil::LinkNode<Agent>* node = agents_.head();
         node != agents_.end();) {
      node->value()->reset(ElementTp(), nullptr);
      tutil::LinkNode<Agent>* const saved_next = node->next();
      node->RemoveFromList();
      node = saved_next;
    }
  }

 private:
  AgentId id_;
  BinaryOp op_;
  ResultTp global_result_;
  ResultTp result_identity_;
  ElementTp element_identity_;
  mutable std::mutex lock_;
  tutil::LinkedList<Agent>
      agents_;  // A double linked list contained all using agents.
};

}  // namespace detail
}  // namespace tvar
}  // namespace tesla

#endif  // TESLA_TVAR_DETAIL_COMBINER_H_
