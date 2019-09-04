#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#include <gflags/gflags.h>
#include <bvar/bvar.h>
#include <brpc/server.h>
#include <butil/time.h>

#include "log/logging.h"

using namespace tesla::log;

bvar::LatencyRecorder  GetterTimeRecorder("test", "rw_lock_data");
int kWorkTimeus = 100;

pthread_rwlock_t kRWLock;

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
  butil::Timer timer;

  while (kStopFlag.load() == false) {
    //int work_times = kWorkTimeus;
    timer.start();

    pthread_rwlock_rdlock(&kRWLock);
    //while (work_times--) {
      kGlobalConfData->name();
    //}
    pthread_rwlock_unlock(&kRWLock);

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
  
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);
  pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
  pthread_rwlock_init(&kRWLock, &attr);
  pthread_rwlockattr_destroy(&attr);

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

    pthread_rwlock_wrlock(&kRWLock);
    kGlobalConfData->set_name(new_name);
    pthread_rwlock_unlock(&kRWLock);
    
    LOG_INFO << "in main:: Modify to new name:" << new_name;
    sleep(1);
  }

  kStopFlag.store(true);
  for (auto& t : thread_group) {
    if (t.joinable()) {
      t.join();
    }
  }

  pthread_rwlock_destroy(&kRWLock);

  return 0;
}
