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

class LockGuard {
public:
  LockGuard(Mutex &mutex) : mutex_(mutex) { mutex_.Lock(); }
  ~LockGuard() { mutex_.UnLock(); }

private:
  Mutex &mutex_;
};
}  // namespace MyCoroutine
