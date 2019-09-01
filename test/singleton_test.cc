#include "log/logging.h"

#include <bvar/bvar.h>
#include <butil/time.h>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "tutil/get_leaky_singleton.h"
#include "tutil/string.h"

using namespace tesla::tutil;
using namespace tesla::log;

static constexpr int kMaxThreadNum = 100;
void* kObjectPointer[kMaxThreadNum];
bvar::IntRecorder GetTimesRecorder[kMaxThreadNum];

class MyClass {

};

void GetObjectPointer(int index, long times) {
  butil::Timer timer;
  while (times--) {
    timer.start();
    GetLeakySingleton<MyClass>();
    timer.stop();
    GetTimesRecorder[index] << timer.n_elapsed();
  }
  kObjectPointer[index] = &GetLeakySingleton<MyClass>();
  LOG_INFO << "in thread " << index << ": " << GetTimesRecorder[index].get_description() << "ns";
}

int main(int argc, char* argv[])
{
  int ret = 0;
  int thread_num = 4;
  long run_times = 1;
  long get_times = 1;
  std::vector<std::thread> group;
  bvar::IntRecorder timer_interval;

  if (argc > 1) {
    ret = StrToInt(argv[1], 10, &thread_num);
    if (ret < 0) {
      LOG_ERROR << "Fail to get thread_num";
      return ret;
    }

    if (ret > kMaxThreadNum) {
      LOG_ERROR << "thread_num was out of range";
      return ERANGE;
    }

    if (argc > 2) {
      ret = StrToLong(argv[2], 10, &run_times);
      if (ret < 0) {
        LOG_ERROR << "Fail to get run_times";
        return ret;
      }

      if (argc > 3) {
        ret = StrToLong(argv[2], 10, &get_times);
        if (ret < 0) {
          LOG_ERROR << "Fail to get get_times";
          return ret;
        }
      }
    }
  }

    for (int i = 0; i < run_times; i++) {
    group.clear();
    for (int i = 0; i < thread_num; i++) {
      kObjectPointer[i] = NULL;
    }

    GetLeakySingleton<MyClass>();
    for (int i = 0; i < thread_num; i++) {
      group.push_back(std::thread(GetObjectPointer, i, get_times));
    }

    for (auto& t : group) {
      if (t.joinable()) {
        t.join();
      }
    }

    // check
    for (int i = 0; i < thread_num - 1; i++) {
      assert(kObjectPointer[i] == kObjectPointer[i + 1]);
    }
  }

  return 0;
}
