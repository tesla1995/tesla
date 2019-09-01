#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#include <bvar/bvar.h>
#include <brpc/server.h>
#include <butil/time.h>

#include "log/logging.h"
#include "tutil/synchronization/fast_pthread_mutex.h"

using namespace tesla::log;
using namespace tesla::tutil;

bvar::LatencyRecorder  GetterTimeRecorder("test", "rw_lock_data");
int kWorkTimeus = 100;

FastPthreadMutex kGlobalMutex;

class TestClass {
 public:
  TestClass(int version = 1, const std::string& name = "tesla")
      : version_(version),
        name_(name) {}

  ~TestClass() {}
 public:
  void set_version(int version) {
    version_ = version;
  }

  void set_name(const std::string& name) {
    name_ = name;
  }

  int version() const {
    return version_;
  }

  const std::string& name() const {
    return name_;
  }
 private:
  int version_{0};
  std::string name_;
};

std::atomic_bool kStopFlag{false};
TestClass* kGlobalConfData = NULL;

void Read() {
  std::string last_name("tesla");
  int work_times = kWorkTimeus;
  butil::Timer timer;

  while (kStopFlag.load() == false) {
    work_times = kWorkTimeus;
    timer.start();

    {
      std::lock_guard guard(kGlobalMutex);
      while (work_times--) {
        kGlobalConfData->name();
      }
    }

    timer.stop();
    GetterTimeRecorder << timer.u_elapsed();
  }
}

int main(int argc, char* argv[])
{
  size_t thread_num = 1;
  size_t sleep_time = 10;
  std::vector<std::thread> thread_group;
  char new_name[64];

  TestClass data;
  kGlobalConfData = &data;

  brpc::StartDummyServerAt(8888/*port*/);
  //google::ParseCommandLineFlags(&argc, &argv, true);

  if (argc > 1) {
    thread_num = atoi(argv[1]);
  }

  if (argc > 2) {
    sleep_time = atoi(argv[2]);
  }

  if (argc > 3) {
    kWorkTimeus = atoi(argv[3]);
  }

  for (size_t i = 0; i < thread_num; i++) {
    thread_group.push_back(std::thread(Read));
  }

  while (sleep_time--) {
    snprintf(new_name, sizeof(new_name), "test%zd", sleep_time);

    {
      std::lock_guard guard(kGlobalMutex);
      kGlobalConfData->set_name(new_name);
    }

    LOG_INFO << "in main:: Modify to new name:" << new_name;
    sleep(1);
  }

  kStopFlag.store(true);
  for (auto& t : thread_group) {
    if (t.joinable()) {
      t.join();
    }
  }

  return 0;
}
