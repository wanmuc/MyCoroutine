#pragma once

#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
// 协程互斥锁
class Mutex {
public:
  Mutex(Schedule *schedule);
  ~Mutex();

  void Lock();
  void UnLock();

private:
  CoMutex co_mutex_;
  Schedule *schedule_;
};
}  // namespace MyCoroutine
