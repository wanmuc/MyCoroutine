#pragma once

#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
// 协程条件变量类封装
class ConditionVariable {
public:
  ConditionVariable(Schedule &schedule) : schedule_(schedule) {
    schedule_.CoCondInit(&co_cond_);
  }
  ~ConditionVariable() { schedule_.CoCondClear(&co_cond_); }

  void NotifyOne();
  void NotifyAll();
  void Wait(std::function<bool()> pred);

private:
  CoCond co_cond_;
  Schedule &schedule_;
};
} // namespace MyCoroutine