#include "mutex.h"

namespace MyCoroutine {
void Schedule::CoMutexInit(CoMutex& co_mutex) {
  co_mutex.lock = false;
  co_mutex.hold_cid = slave_cid_;
  assert(mutexs_.find(&co_mutex) == mutexs_.end());
  mutexs_.insert(&co_mutex);
}

void Schedule::CoMutexClear(CoMutex &co_mutex) {
  mutexs_.erase(&co_mutex);
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

bool Schedule::CoMutexTryLock(CoMutex &co_mutex) {
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

void Schedule::CoMutexResume() {
  assert(is_master_);
  for (auto * mutex : mutexs_) {
    if (mutex->lock) continue;  // 锁没释放，不需要唤醒其他从协程
    if (mutex->suspend_cid_list.size() <= 0) continue;  // 锁已经释放了，但是没有挂起的从协程，也不需要唤醒
    int32_t cid = mutex->suspend_cid_list.front();
    mutex->suspend_cid_list.pop_front();
    mutex->suspend_cid_list.erase(cid);
    CoroutineResume(cid);  // 每次只能唤醒等待队列中的一个从协程，采用先进先出的策略
  }
}

Mutex::Mutex(Schedule& schedule) : schedule_(schedule) {
  schedule_.CoMutexInit(co_mutex_);
}

Mutex::~Mutex() { schedule_.CoMutexClear(co_mutex_); }

void Mutex::Lock() { schedule_.CoMutexLock(co_mutex_); }

void Mutex::UnLock() { schedule_.CoMutexUnLock(co_mutex_); }

bool Mutex::TryLock() { return schedule_.CoMutexTryLock(co_mutex_); }
}  // namespace MyCoroutine
