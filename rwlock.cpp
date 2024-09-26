#include "rwlock.h"

namespace MyCoroutine {
void Schedule::CoRWLockInit(CoRWLock &rwlock) {
  rwlock.lock_state = RWLockState::kUnLock;
  rwlock.hold_write_cid = kInvalidCid;
  assert(rwlocks_.find(&rwlock) == rwlocks_.end());
  rwlocks_.insert(&rwlock);
}

void Schedule::CoRWLockClear(CoRWLock &rwlock) { rwlocks_.erase(&rwlock); }

void Schedule::CoRWLockWrLock(CoRWLock &rwlock) {
  while (true) {
    assert(not is_master_);
    if (rwlock.lock_state == RWLockState::kUnLock) {  // 无锁状态，直接加写锁
      rwlock.lock_state = RWLockState::kWriteLock;
      rwlock.hold_write_cid = slave_cid_;
      return;
    }
    // 不可重入，同一个从协程只能锁一次写锁，不能锁定多次
    assert(rwlock.hold_write_cid != slave_cid_);
    // 更新因为等待读写锁而被挂起的从协程信息
    auto iter = find(rwlock.suspend_list.begin(), rwlock.suspend_list.end(),
                     pair<RWLockState, int32_t>(RWLockState::kWriteLock, slave_cid_));
    if (iter == rwlock.suspend_list.end()) {
      rwlock.suspend_list.push_back(pair<RWLockState, int32_t>(RWLockState::kWriteLock, slave_cid_));
    }
    // 从协程让出执行权
    CoroutineYield();
  }
}
void Schedule::CoRWLockWrUnLock(CoRWLock &rwlock) {
  // TODO
}
void Schedule::CoRWLockRdLock(CoRWLock &rwlock) {
  // TODO
}
void Schedule::CoRWLockRdUnLock(CoRWLock &rwlock) {
  // TODO
}
}  // namespace MyCoroutine
