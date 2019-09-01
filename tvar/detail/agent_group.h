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
// Date: Wed Feb  6 10:14:39 CST 2019

#ifndef TESLA_TVAR_DETAIL_AGENT_GROUP_H_
#define TESLA_TVAR_DETAIL_AGENT_GROUP_H_

#include <stdlib.h>
#include <deque>
#include <mutex>
#include <memory>
#include <vector>

#include "tutil/compiler_specific.h"
#include "log/logging.h"

namespace tesla {
namespace tvar {
namespace detail {

using AgentId = int;
using AgentIdQueue = std::deque<AgentId>;
using AgentIdQueuePtr = std::unique_ptr<AgentIdQueue>;

template <typename Agent>
class AgentGroup {
 public:
  using agent_type = Agent;

  constexpr static size_t kRawBlockSize = 4096;
  constexpr static size_t kElementPerBlock =
    (kRawBlockSize + sizeof(Agent) - 1) / sizeof(Agent); /* ceil */

  // we construct all agents when initializing ThreadBlock, which has side effects:
  //  * may return non-null for unexist id.
  //  * lifetime of agent is more complex. User has to reset the agent before
  //    destroying id otherwise when the agent is reused by another one who
  //    gets the reused id, things are screwed.
  struct TESLA_CACHELINE_ALIGNMENT ThreadBlock {
    inline Agent* at(size_t offset) {
      return agents_ + offset;
    }

   private:
    Agent agents_[kElementPerBlock];
  };

  inline static AgentId CreateNewAgent() {
    std::lock_guard<std::mutex> lock(s_mutex_);
    AgentId agent_id = 0;
    if (!GetFreeIds().empty()) {
      // Last in first out to improve cache hit.
      agent_id = GetFreeIds().back();
      GetFreeIds().pop_back();
    } else {
      agent_id = s_agent_kinds_++; 
    }
    return agent_id;
  }

  inline static int DestroyAgent(AgentId id) {
    std::lock_guard<std::mutex> lock(s_mutex_);
    if (id < 0 || id >= s_agent_kinds_) {
      errno = EINVAL;
      return -1;
    }
    GetFreeIds().push_back(id);
    return 0;
  }

  // Calculates location of the agent by its id:
  //   block_id = id / kElementPerBlock;
  //   agent_id = id - block_id * kElementPerBlock;
  inline static Agent* GetTlsAgent(AgentId id) {
    if (TESLA_LIKELY(id >= 0)) {
      if (s_tls_blocks_) {
        const size_t block_id = static_cast<size_t>(id) / kElementPerBlock;
        if (block_id < s_tls_blocks_->size()) {
          std::unique_ptr<ThreadBlock>& block = (*s_tls_blocks_)[block_id];       
          if (block) {
            return block->at(id - block_id * kElementPerBlock);
          }
        }
      } 
    }
    return nullptr;
  }

  // Note: May return non-null for unexist id, see notes on ThreadBlock.
  inline static Agent* GetOrCreateTlsAgent(AgentId id) {
    if (TESLA_UNLIKELY(id < 0)) {
      LOG_ERROR << "Invalid id=" << id;
      return NULL;
    }

    if (s_tls_blocks_ == nullptr) {
      s_tls_blocks_ = std::make_unique<std::vector<std::unique_ptr<ThreadBlock>>>();
      if (__builtin_expect(s_tls_blocks_ == nullptr, 0)) {
        LOG_ERROR << "Fail to create vector";
        return NULL;
      }
      //LOG_INFO << "The address of s_tls_blocks_ is " << s_tls_blocks_.get();
    }

    const size_t block_id = size_t(id) / kElementPerBlock;
    if (block_id >= s_tls_blocks_->size()) {
      // The 32ul avoid pointless small resizes.
      s_tls_blocks_->resize(std::max(block_id + 1, 32ul));
    }
    std::unique_ptr<ThreadBlock>& block = (*s_tls_blocks_)[block_id];
    if (block == nullptr) {
      std::unique_ptr<ThreadBlock> new_block = std::make_unique<ThreadBlock>();
      if (__builtin_expect(new_block == nullptr, 0)) {
        LOG_ERROR << "Fail to create thread block";
        return NULL;
      }
      block = std::move(new_block);
      //LOG_INFO << "The address of block is " << block.get();
    }
    return block->at(id - block_id * kElementPerBlock);
  }

 private:

  inline static AgentIdQueue& GetFreeIds() {
    if (__builtin_expect(s_free_ids_ == nullptr, 0)) {
      s_free_ids_.reset(new AgentIdQueue());
      if (s_free_ids_ == nullptr) {
        abort();
      }
    }
    return *s_free_ids_;
  }
  
  using ThreadBlockPtr = std::unique_ptr<ThreadBlock>;
  using ThreadBlockVectorPtr = std::unique_ptr<std::vector<ThreadBlockPtr>>;

  static std::mutex                           s_mutex_;
  static AgentId                              s_agent_kinds_;
  static AgentIdQueuePtr                      s_free_ids_;
  static thread_local ThreadBlockVectorPtr    s_tls_blocks_;
};

template <typename Agent>
std::mutex AgentGroup<Agent>::s_mutex_;

template <typename Agent>
AgentId AgentGroup<Agent>::s_agent_kinds_ = 0;

template <typename Agent>
AgentIdQueuePtr AgentGroup<Agent>::s_free_ids_ = nullptr;

template <typename Agent>
thread_local typename AgentGroup<Agent>::ThreadBlockVectorPtr
AgentGroup<Agent>::s_tls_blocks_ = nullptr;

} // namespace detail
} // namespace tvar
} // namespace tesla

#endif // TESLA_TVAR_DETAIL_AGENT_GROUP_H_
