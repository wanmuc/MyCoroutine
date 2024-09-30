#pragma once

#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
// 协程CallOnce
class CallOnce {
 public:
  CallOnce(Schedule &schedule) : schedule_(schedule) { schedule_.CoCallOnceInit(co_callonce_); }
  ~CallOnce() { schedule_.CoCallOnceClear(co_rwlock_); }

  template <typename Function, typename... Args>
  void Do(Function &&func, Args &&...args) {
    schedule_.CoCallOnceDo(co_callonce_, std::forward<Function>(func), std::forward<Args>(args)...);
  }

 private:
  CoCallOnce co_callonce_;
  Schedule &schedule_;
};
}  // namespace MyCoroutine
