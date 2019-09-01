#ifndef TESLA_TUTIL_CONTAINERS_DOUBLY_BUFFERED_DATA_H_
#define TESLA_TUTIL_CONTAINERS_DOUBLY_BUFFERED_DATA_H_

#include <pthread.h>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <type_traits>

#include "tutil/macros.h"
#include "tutil/compiler_specific.h"
#include "tutil/current_thread.h"
#include "log/logging.h"

namespace tesla {
namespace tutil {

template <typename T>
void DeleteObject(void* p) {
  delete static_cast<T*>(p);
}

template <typename T>
class DoublyBufferedData {
 private:
  class Wrapper;
 public:
  using ModifyCallback = std::function<int (T&)>;

  class ScopedPtr {
   friend class DoublyBufferedData;
   public:
    ScopedPtr() {}

    ~ScopedPtr() {
      if (w_) {
        w_->EndRead();
      }
    }

    DISALLOW_COPY_AND_ASSIGN(ScopedPtr);

   public:
    T& operator*() {
      return *data_;
    }

    const T* operator->() {
      return data_;
    }

    const T* get() {
      return data_;
    }
   private:
    // The read thread CAN NOT modify data.
    const T* data_{nullptr};
    Wrapper* w_{nullptr};
  };

 public:
  DoublyBufferedData();
  ~DoublyBufferedData();
 public:
  int Read(ScopedPtr& ptr);
  int Modify(ModifyCallback callback);
 public:
  Wrapper* AddWrapper();
  void RemoveWrapper(Wrapper* w);
 private:
  // Foreground and background data.
  T data_[2];

  // Index of foreground instance.
  std::atomic<int> foreground_index_;

  // Key to access thread_local wrappers_.
  bool create_key_;
  pthread_key_t wrapper_key_;

  // All thread_local instance. 
  std::vector<Wrapper*> wrappers_;

  // Sequence access to wrappers_.
  std::mutex wrappers_mutex_;

  // Sequence modification.
  std::mutex modify_mutex_;
};

template <typename T>
class DoublyBufferedData<T>::Wrapper {
 friend class DoublyBufferedData;
 public:
  explicit Wrapper(DoublyBufferedData* control)
      : control_(control) {}

  ~Wrapper() {
    if (control_) {
      control_->RemoveWrapper(this);
    }
  }

 public:
  void BeginRead() {
    mutex_.lock();
  }

  void EndRead() {
    mutex_.unlock();
  }

  void WaitDone() {
    std::lock_guard<std::mutex> guard(mutex_);
  }

 private:
  DoublyBufferedData* control_{nullptr};
  std::mutex mutex_;
};

template <typename T>
DoublyBufferedData<T>::DoublyBufferedData()
    : foreground_index_(0),
      create_key_(false) {
  wrappers_.reserve(64);

  int rc = pthread_key_create(&wrapper_key_, DeleteObject<Wrapper>);
  if (rc != 0) {
    LOG_ERROR << "in DoublyBufferedData:: pthread_key_create fail:" << tesla::log::strerror_tl(errno);
  } else {
    create_key_ = true;
  }

  // Initialize _data for some POD types. This is essential for pointer
  // types because they should be Read() as NULL before any Modify().
  if (std::is_pod<T>::value) {
    data_[0] = T();
    data_[1] = T();
  }
}

template <typename T>
DoublyBufferedData<T>::~DoublyBufferedData() {
  if (create_key_) {
    pthread_key_delete(wrapper_key_);
  }

  {
    std::lock_guard<std::mutex> guard(wrappers_mutex_);
    for (size_t i = 0; i < wrappers_.size(); ++i) {
      wrappers_[i]->control_ = nullptr;
      delete wrappers_[i];
    }
    wrappers_.clear();
  }
}

template <typename T>
int DoublyBufferedData<T>::Read(ScopedPtr& ptr) {
  if (TESLA_UNLIKELY(create_key_ == false)) {
    return -1;
  }

  Wrapper* wrapper = reinterpret_cast<Wrapper*>(pthread_getspecific(wrapper_key_));
  if (TESLA_LIKELY(wrapper != nullptr)) {
    wrapper->BeginRead();
    ptr.w_ = wrapper;
    ptr.data_ = data_ + foreground_index_.load(std::memory_order_acquire);
    return 0;
  }

  wrapper = AddWrapper();
  if (TESLA_LIKELY(wrapper != nullptr)) {
    const int rc = pthread_setspecific(wrapper_key_, wrapper);
    if (rc == 0) {
      wrapper->BeginRead();
      ptr.w_ = wrapper;
      ptr.data_ = data_ + foreground_index_.load(std::memory_order_acquire);
      return 0;
    }
  }

  return -1;
}

template <typename T>
int DoublyBufferedData<T>::Modify(ModifyCallback callback) {
  std::lock_guard<std::mutex> guard(modify_mutex_);

  // Modify background data.
  int background_index = !foreground_index_.load(std::memory_order_relaxed);
  const int ret = callback(data_[background_index]);
  if (ret < 0) {
    LOG_ERROR << "in Modify:: Fail to modify background data";
    return -1;
  }

  // Flip foreground and background.
  foreground_index_.store(background_index, std::memory_order_release);
  background_index = !background_index; 

  // Walk through wrappers_ to identify that all threads in process see the new
  // foreground data and no thread is accessing new background data.
  {
    std::lock_guard<std::mutex> guard(wrappers_mutex_);
    for (size_t i = 0; i < wrappers_.size(); i++) {
      wrappers_[i]->WaitDone();
    }
  }

  // Modify new background data, i.e. old foreground data.
  const int ret2 = callback(data_[background_index]);
  if (ret2 < 0 || ret2 != ret) {
    LOG_ERROR << "in Modify:: Fail to modify background data(old foreground data)";
    return -1;
  }

  return 0;
}

template <typename T>
typename DoublyBufferedData<T>::Wrapper*
DoublyBufferedData<T>::AddWrapper() {
  std::unique_ptr<Wrapper> w = std::make_unique<Wrapper>(this);
  if (w == nullptr) {
    LOG_ERROR << "in AddWrapper:: make_unique";
    return nullptr;
  }

  {
    std::lock_guard<std::mutex> guard(wrappers_mutex_);
    wrappers_.push_back(w.get());
  }

  return w.release();
}

template <typename T>
void DoublyBufferedData<T>::RemoveWrapper(Wrapper* w) {
  if (w == nullptr) {
    return;
  }

  {
    std::lock_guard<std::mutex> guard(wrappers_mutex_);
    for (size_t i = 0; i < wrappers_.size(); i++) {
      if (wrappers_[i] == w) {
        std::swap(wrappers_[i], wrappers_.back());
        wrappers_.pop_back();
      }
    }
  }
  return;
}

}  // namespace tutil
}  // namespace tesla

#endif  // TESLA_TUTIL_CONTAINERS_DOUBLY_BUFFERED_DATA_H_
