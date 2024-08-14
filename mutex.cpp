#include "mutex.h"

namespace MyCoroutine {
void Schedule::CoMutexInit(CoMutex& co_mutex) {
  co_mutex.id = mutex_manage_.alloc_id++;
  co_mutex.lock = false;
  co_mutex.hold_cid = slave_cid_;
  assert(mutex_manage_.mutexs.find(co_mutex.id) == mutex_manage_.mutexs.end());
  mutex_manage_.mutexs[co_mutex.id] = &co_mutex;
}

void Schedule::CoMutexClear(CoMutex &co_mutex) {
  mutex_manage_.mutexs.erase(co_mutex.id);
}

void Schedule::CoMutexLock(CoMutex& co_mutex) {
  while (true) {
    assert(not is_master_);
    if (not co_mutex.lock) {
      co_mutex.lock = true;  // 加锁成功，直接返回
      co_mutex.hold_cid = slave_cid_;
      return;
    }
    // 不可重入，同一个从协程只能锁定一次，不能锁定多次
    assert(co_mutex.hold_cid != slave_cid_);
    // 更新因为等待互斥锁而被挂起的从协程id
    auto iter = find(co_mutex.suspend_cid_list.begin(), co_mutex.suspend_cid_list.end(), slave_cid_);
    if (iter == co_mutex.suspend_cid_list.end()) {
      co_mutex.suspend_cid_list.push_back(slave_cid_);
    }
    // 从协程让出执行权
    CoroutineYield();
  }
}

void Schedule::CoMutexTryLock(CoMutex &co_mutex) {
  assert(not is_master_);
  if (not co_mutex.lock) {
    co_mutex.lock = true; // 加锁成功，直接返回
    co_mutex.hold_cid = slave_cid_;
    return true;
  }
  return false;
}

void Schedule::CoMutexUnLock(CoMutex& co_mutex) {
  assert(not is_master_);
  assert(co_mutex.lock);  // 必须是锁定的
  co_mutex.lock = false;  // 设置成false即可，后续由调度器schedule去激活那些被挂起的从协程
  co_mutex.hold_cid = kInvalidCid;
}

Mutex::Mutex(Schedule& schedule) : schedule_(schedule) {
  schedule_.CoMutexInit(co_mutex_);
}

Mutex::~Mutex() { schedule_.CoMutexClear(co_mutex_); }

void Mutex::Lock() { schedule_.CoMutexLock(co_mutex_); }

void Mutex::UnLock() { schedule_.CoMutexUnLock(co_mutex_); }

bool Mutex::TryLock() { return schedule_.CoMutexTryLock(co_mutex_); }
}  // namespace MyCoroutine
