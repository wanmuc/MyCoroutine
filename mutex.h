#pragma once

#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
// 协程互斥锁
class Mutex {
public:
  Mutex(Schedule &schedule) : schedule_(schedule) {
    schedule_.CoMutexInit(co_mutex_);
  }
  ~Mutex() { schedule_.CoMutexClear(co_mutex_); }

  void Lock();
  void UnLock();
  bool TryLock();

private:
  CoMutex co_mutex_;
  Schedule &schedule_;
};

// 自动加解锁类封装
class LockGuard {
public:
  LockGuard(Mutex &mutex) : mutex_(mutex) { mutex_.Lock(); }
  ~LockGuard() { mutex_.UnLock(); }

private:
  Mutex &mutex_;
};
}  // namespace MyCoroutine
