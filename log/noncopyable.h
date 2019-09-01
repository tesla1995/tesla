#ifndef TESLALOG_NONCOPYABLE_H_
#define TESLALOG_NONCOPYABLE_H_

namespace tesla {
namespace log {

class Noncopyable {
 protected: 
  Noncopyable() = default;
  ~Noncopyable() = default;

  Noncopyable(const Noncopyable&) = delete;
  Noncopyable& operator=(const Noncopyable &) = delete;
};

} // namespace log
} // namespace tesla

#endif // TESLALOG_NONCOPYABLE_H_
