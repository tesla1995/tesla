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
// Date: Fri Nov 15 22:59:57 CST 2019

#include "allocator/metadata_allocator.h"

#include <cstddef>
#include <cstdint>

#include <mutex>
#include <atomic>

#include "allocator/system_alloc.h"

static std::mutex kMetadataLock;
static std::atomic<uint64_t> metadata_system_bytes_{0};

static constexpr size_t kMetadataAllocChunkSize = 8 * 1024 * 1024;
static constexpr size_t kMetadataAlignment = BASE_ALIGNMENT_SIZE;

static char *metadata_chunk_alloc_{NULL};
static size_t metadata_chunk_avail_{0};

namespace tesla {
namespace allocator {

void* MetaDataAlloc(size_t bytes)
{
  if (bytes >= kMetadataAllocChunkSize) {
    void* result = TeslaMalloc_SystemAlloc(bytes, NULL, kMetadataAlignment);
    if (result != NULL) {
      // Note that the actual size of `result' may be larger than `bytes'.
      metadata_system_bytes_.fetch_add(bytes);
    }
    return result;
  }

  std::lock_guard<std::mutex> guard(kMetadataLock);

  intptr_t alignment = -reinterpret_cast<intptr_t>(metadata_chunk_alloc_) & (kMetadataAlignment - 1);
  if (metadata_chunk_avail_ < alignment + bytes) {
    size_t actual_size = 0;
    void* result = TeslaMalloc_SystemAlloc(kMetadataAllocChunkSize,
                                           &actual_size, kMetadataAlignment);
    if (result == NULL) {
      return NULL;
    }

    metadata_chunk_alloc_ = reinterpret_cast<char*>(result);
    metadata_chunk_avail_ = actual_size;

    alignment = 0;
  }

  void* result = reinterpret_cast<void*>(metadata_chunk_alloc_ + alignment);
  bytes += alignment;
  metadata_chunk_alloc_ += bytes;
  metadata_chunk_avail_ -= bytes;
  metadata_system_bytes_.fetch_add(bytes);
  return result;
}

uint64_t metadata_system_bytes()
{
  return metadata_system_bytes_.load(std::memory_order_relaxed);
}

}  // namespace allocator
}  // namespace tesla
