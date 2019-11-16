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
// Date: Thu Nov 14 22:50:08 CST 2019

#include "allocator/system_alloc.h"

#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <mutex>
#include <atomic>

namespace tesla {
namespace allocator {

static size_t kPageSize = 0;
static size_t kPageMask = 0;

// Number of bytes taken from system.
std::atomic<uint64_t> TeslaMalloc_Taken_{0};

void* MmapMalloc_SystemAlloc(size_t size, size_t *actual_size,
        size_t alignment)
{
  // Enforce page alignment.
  if (kPageSize == 0) kPageSize = getpagesize();
  if (alignment < kPageSize) alignment = kPageSize;
  size_t aligned_size = ((size + alignment - 1) / alignment) * alignment;
  // Discard requests that overflow.
  if (aligned_size < size) {
    return NULL;
  }
  size = aligned_size;

  if (actual_size) {
    *actual_size = size;
  }

  size_t extra = 0;
  if (alignment > kPageSize) {
    extra = alignment - kPageSize;
  }

  void* result = mmap(NULL, size + extra, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (result == reinterpret_cast<void*>(MAP_FAILED)) {
    return NULL;
  }

  size_t adjust = 0;
  uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
  if ((ptr & (alignment - 1)) == 0) {
    return result;
  }

  adjust = alignment - (ptr & (alignment - 1));
  if (adjust) {
    munmap(reinterpret_cast<char*>(ptr), adjust);
  }

  if (adjust < extra) {
    munmap(reinterpret_cast<char*>(ptr + adjust + size), extra - adjust);
  }

  ptr += adjust;
  return reinterpret_cast<char*>(ptr);
}

void* TeslaMalloc_SystemAlloc(size_t size, size_t *actual_size,
        size_t alignment)
{
  if (size + alignment < size) {
    return NULL;
  }
  
  size_t actual_size_storage = 0;
  if (actual_size == NULL) {
    actual_size = &actual_size_storage;
  }

  // Enforce miniment alignment
  if (alignment < BASE_ALIGNMENT_SIZE) alignment = BASE_ALIGNMENT_SIZE;

  void* result = MmapMalloc_SystemAlloc(size, actual_size, alignment);
  if (result != NULL) {
    // TODO(qiuy): use memory_order_relaxed instead ?
    TeslaMalloc_Taken_.fetch_add(*actual_size);
  }
  return result;
}

bool TeslaMalloc_SystemRelease(void* addr, size_t length)
{
  if (kPageSize == 0) kPageSize = getpagesize();
  if (kPageMask == 0) kPageMask = kPageSize - 1;

  uintptr_t start = reinterpret_cast<uintptr_t>(addr);
  uintptr_t end = start + length;

  uintptr_t new_start = (start + kPageMask) & (~kPageMask);
  uintptr_t new_end = end & (~kPageMask);

  assert((new_start & kPageMask) == 0);
  assert((new_end & kPageMask) == 0);
  assert(new_start >= start);
  assert(new_end <= end);

  if (new_end > new_start) {
    int result = -1;
    do {
      result = madvise(reinterpret_cast<void*>(new_start),
               new_end - new_start, MADV_DONTNEED);
    } while (result == -1 && errno == EAGAIN);

    return result != -1;
  }
  return false;
}

uint64_t TeslaMalloc_Taken()
{
  // TODO(qiuy): use memory_order_relaxed instead?
  return TeslaMalloc_Taken_.load();
}

}  // namespace allocator
}  // namespace tesla
