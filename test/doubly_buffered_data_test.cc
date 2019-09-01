#include "tutil/containers/doubly_buffered_data.h"

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#include <gflags/gflags.h>
#include <bvar/bvar.h>
#include <brpc/server.h>
#include <butil/time.h>

#include "log/logging.h"

using namespace tesla::tutil;
using namespace tesla::log;

bvar::LatencyRecorder  GetterTimeRecorder("test", "doubly_buffered_data");
int kWorkTimeus = 10000;

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

DoublyBufferedData<TestClass> * kGlobalConfData = NULL;
std::atomic_bool kStopFlag{false};

void Read() {
  std::string last_name("tesla");
  butil::Timer timer;

  while (kStopFlag.load() == false) {
    //int work_time = kWorkTimeus;
    timer.start();
    {
      DoublyBufferedData<TestClass>::ScopedPtr ptr;
      int ret = kGlobalConfData->Read(ptr);
      if (ret != 0) {
        LOG_ERROR << "Read";
        return;
      }

      //while (work_time--) {
        ptr->name();
      //}
    }
    timer.stop();
    GetterTimeRecorder << timer.u_elapsed();
  }
}

void Modify() {
  auto f = [](TestClass& data) -> int {
    data.set_name("test");
    return 0;
  };

  kGlobalConfData->Modify(f);

  LOG_INFO << "Modify data";
}

int main(int argc, char* argv[])
{
  int ret = 0;
  size_t thread_num = 1;
  size_t sleep_time = 10;
  std::vector<std::thread> thread_group;
  char new_name[64];

  DoublyBufferedData<TestClass> data;
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

  {
    DoublyBufferedData<TestClass>::ScopedPtr ptr;

    ret = data.Read(ptr);
    if (ret != 0) {
      LOG_ERROR << "Read";
      return ret;
    }

    LOG_INFO << "version:" << ptr->version() << ", name:" << ptr->name();
  }

  for (size_t i = 0; i < thread_num; i++) {
    thread_group.push_back(std::thread(Read));
  }

  while (sleep_time--) {
    snprintf(new_name, sizeof(new_name), "test%zd", sleep_time);

    auto f = [&](TestClass& data) -> int {
      data.set_name(new_name);
      return 0;
    };

    kGlobalConfData->Modify(f);

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
