#include <pthread.h>
#include <iostream>
#include <bvar/bvar.h>
#include <gflags/gflags.h>

bvar::LatencyRecorder g_mutex_latency("posix_lock", "mutex");

int main(int argc, char* argv[])
{
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  butil::Timer timer;

  google::ParseCommandLineFlags(&argc, &argv, true/*表示把识别的参数从argc/argv中删除*/);

  for (size_t i = 0; i < 100000000; i++) {
    timer.start();
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    timer.stop();
    g_mutex_latency << timer.n_elapsed();
  }

  return 0;
}
