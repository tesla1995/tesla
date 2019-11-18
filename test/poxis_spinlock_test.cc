#include <pthread.h>
#include <iostream>
#include <bvar/bvar.h>
#include <gflags/gflags.h>

bvar::LatencyRecorder g_spinlock_latency("posix_lock", "spinlock");

int main(int argc, char* argv[])
{
  pthread_spinlock_t spinlock;
  butil::Timer timer;

  google::ParseCommandLineFlags(&argc, &argv, true/*表示把识别的参数从argc/argv中删除*/);

  pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
  for (size_t i = 0; i < 100000000; i++) {
    timer.start();
    pthread_spin_lock(&spinlock);
    pthread_spin_unlock(&spinlock);
    timer.stop();
    g_spinlock_latency << timer.n_elapsed();
  }
  pthread_spin_destroy(&spinlock);
  return 0;
}
