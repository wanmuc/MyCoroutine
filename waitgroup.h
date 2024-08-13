#pragma once
#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
class WaitGroup {
public:
  WaitGroup(Schedule *schedule) : schedule_(schedule) {
    bid_ = schedule_->BatchCreate();
  }
  template <typename Function, typename... Args>
  void Add(Function &&func, Args &&...args) {
    schedule_->BatchAdd(bid_, std::forward<Function>(func),
                        std::forward<Args>(args)...);
  }
  void Wait() { schedule_->BatchRun(bid_); }

private:
  int bid_; // Batch的id
  Schedule *schedule_{nullptr};
};
} // namespace MyCoroutine