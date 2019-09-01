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
// Date: Sat Jul  6 19:41:18 CST 2019

#include <unistd.h>
#include <vector>
#include <thread>

#include "wait_free/lifo.h"
#include "tutil/timestamp.h"

using namespace tesla::tutil;
using namespace tesla::wait_free;

struct StackValue {
  int64_t a{0};
  int64_t b{0};
  int64_t sum{0};
};

struct GlobalConfiguration {
  LockFreeStack<StackValue> stack;
  int64_t loop_times;
  std::atomic<int64_t> producer_count;
};

void thread_consumer(void* data) {
  SetCpuAffinity();

  GlobalConfiguration* global_conf = (GlobalConfiguration*)data;
  StackValue stack_value;
  bool skip = false;
  while (true) {
    if (global_conf->stack.Pop(stack_value)) {
      // Check value
      if ((stack_value.a + stack_value.b) != stack_value.sum) {
        fprintf(stderr, "error data! a[%ld] b[%ld] sum[%ld]\n",
          stack_value.b, stack_value.b, stack_value.sum);
      }
      skip = false;
    } else {
      if (0 == global_conf->producer_count.load()) {
        if (skip) {
          break;
        } else {
          skip = true;
        }
      }
    }
  }
}

void thread_producer(void* data) {
  SetCpuAffinity();

  GlobalConfiguration* global_conf = (GlobalConfiguration*)data;
  //LifoNode<StackValue>* nodes = new LifoNode<StackValue>[global_conf->loop_times];
  int64_t sum_base = GetCurrentThreadId() * global_conf->loop_times;
  StackValue stack_value;
  for (int64_t i = 0; i < global_conf->loop_times; i++) {
    stack_value.a = sum_base + i;
    stack_value.b = i;
    stack_value.sum = sum_base + 2 * i;
    LifoNode<StackValue>* node = new LifoNode<StackValue>(stack_value);
    global_conf->stack.Push(node);
  }
  global_conf->producer_count.fetch_add(-1);
}

void run_test(GlobalConfiguration* global_conf, const int64_t thread_count) {
  std::vector<std::thread> producer_group;
  std::vector<std::thread> consumer_group;

  global_conf->producer_count.store(thread_count);
  Timestamp start = Timestamp::Now();
  for (int64_t i = 0; i < thread_count; i++) {
    producer_group.push_back(std::thread(thread_producer, global_conf));
    consumer_group.push_back(std::thread(thread_consumer, global_conf));
  }
  for (int64_t i = 0; i < thread_count; i++) {
    producer_group[i].join();
    consumer_group[i].join();
  }
  Duration d = Timestamp::Now() - start;
  int64_t push_pop_sum = thread_count * 2 * global_conf->loop_times;
  fprintf(stdout, "threads=%ld+%ld push+pop=%ld timeus=%lf tps=%0.3lftimes/s\n",
          thread_count, thread_count,
          push_pop_sum,
          d.Microseconds(),
          1000000.0 * (double)(push_pop_sum) / (double)(d.Microseconds()));
}

int main(const int argc, char** argv) {
  int64_t cpu_count = 0;
  if (1 < argc) {
    cpu_count = atoi(argv[1]);
  }
  if (0 >= cpu_count) {
    cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
  }
  fprintf(stdout, "cpu_count[%ld]\n", cpu_count);

  int64_t producer_count = (cpu_count + 1) / 2;
  fprintf(stdout, "producer_count[%ld]\n", producer_count);

  int64_t memory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
  int64_t available = memory * 4 / 10;
  fprintf(stdout, "available memory[%lf]G\n", (double)available / 1000000000L);
  int64_t count = available / sizeof(LifoNode<StackValue>) / producer_count;

  GlobalConfiguration g_conf;
  g_conf.loop_times = count;
  fprintf(stdout, "loop_times[%ld]\n", g_conf.loop_times);

  fprintf(stdout, "Run and check pop result...\n");
  run_test(&g_conf, producer_count);

  return 0;
}
