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
// Date: Sat Jul  6 09:53:45 CST 2019

#include "wait_free/util.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <atomic>

namespace tesla {
namespace wait_free {

static_assert(sizeof(std::atomic<uint16_t>) == sizeof(uint16_t),
              "atomic template should not modify the size of data type");

int64_t GetCurrentThreadId() {
  static std::atomic<int64_t> gid(0);
  thread_local static int64_t tid = -1;
  if (tid == -1) {
    tid = gid.fetch_add(1);
  }
  return tid;
}

void SetCpuAffinity() {
  int64_t cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(GetCurrentThreadId() % cpu_count, &cpuset);
  if (0 == pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
    fprintf(stdout, "pthread_setaffinity_np success %ld \n",
      GetCurrentThreadId() % cpu_count);
  } else {
    fprintf(stdout, "pthread_setaffinity_np fail %ld \n",
      GetCurrentThreadId() % cpu_count);
  }
}


}  // wait_free
}  // tesla