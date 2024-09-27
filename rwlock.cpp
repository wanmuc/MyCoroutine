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
    if (rwlock.lock_state == RWLockState::kUnLock) { // 无锁状态，直接加写锁
      rwlock.lock_state = RWLockState::kWriteLock;
      rwlock.hold_write_cid = slave_cid_;
      return;
    }
    if (rwlock.lock_state == RWLockState::kWriteLock) {
      // 不可重入，同一个从协程只能锁一次写锁，不能锁定多次
      assert(rwlock.hold_write_cid != slave_cid_);
    }
    // 更新因为等待读写锁而被挂起的从协程信息（只要有加锁了，加写锁的协程都要做挂起等待）
    auto iter = find(rwlock.suspend_list.begin(), rwlock.suspend_list.end(),
                     {RWLockType::kWrite, slave_cid_});
    if (iter == rwlock.suspend_list.end()) {
      rwlock.suspend_list.push_back({RWLockType::kWrite, slave_cid_});
    }
    // 从协程让出执行权
    CoroutineYield();
  }
}

void Schedule::CoRWLockWrUnLock(CoRWLock &rwlock) {
  assert(not is_master_);
  assert(rwlock.lock_state == RWLockState::kWriteLock); // 必须是写锁锁定
  assert(rwlock.hold_write_cid == slave_cid_); // 必须是持有锁的从协程来释放锁。
  rwlock.lock_state = RWLockState::kUnLock; // 设置成无锁状态即可，后续由调度器schedule去激活那些被挂起的从协程
  rwlock.hold_write_cid = kInvalidCid;

  // 释放锁之后，要判断之前是否在等待队列中，如果是，则需要从等待队列中删除
  auto iter = find(rwlock.suspend_list.begin(), rwlock.suspend_list.end(),
                   {RWLockType::kWrite, slave_cid_});
  if (iter != rwlock.suspend_list.end()) {
    rwlock.suspend_list.erase(iter);
  }
}

void Schedule::CoRWLockRdLock(CoRWLock &rwlock) {
  while (true) {
    assert(not is_master_);
    if (rwlock.lock_state == RWLockState::kUnLock) { // 无锁状态，直接加读锁
      assert(rwlock.hold_read_cid_set.size() == 0);
      rwlock.lock_state = RWLockState::kReadLock;
      rwlock.hold_read_cid_set.insert(slave_cid_);
      return;
    }
    if (rwlock.lock_state == RWLockState::kReadLock) { // 读锁锁定状态，也可以加读锁成功
      // 读锁不可重入，一个协程不可以对同一个读写锁，加多次读锁。
      assert(rwlock.hold_read_cid_set.find(slave_cid_) == rwlock.hold_read_cid_set.end());
      rwlock.hold_read_cid_set.insert(slave_cid_);
      return;
    }
    // 执行到这里，就是写锁锁定状态，需要做挂起等待，等待写锁释放。
    auto iter = find(rwlock.suspend_list.begin(), rwlock.suspend_list.end(),
                     {RWLockType::kRead, slave_cid_});
    if (iter == rwlock.suspend_list.end()) {
      rwlock.suspend_list.push_back({RWLockType::kRead, slave_cid_});
    }
    // 从协程让出执行权
    CoroutineYield();
  }
}

void Schedule::CoRWLockRdUnLock(CoRWLock &rwlock) {
  assert(not is_master_);
  assert(rwlock.lock_state == RWLockState::kReadLock); // 必须是读锁锁定
  assert(rwlock.hold_read_cid_set.find(slave_cid_) !=
         rwlock.hold_read_cid_set.end()); // 必须是持有锁的从协程来释放锁。
  rwlock.lock_state = RWLockState::kUnLock; // 设置成无锁状态即可，后续由调度器schedule去激活那些被挂起的从协程
  // 释放锁之后，则需要从等待队列中删除（可能之前已经在CoRWLockResume中删除了，这里是做必要的清理）
  rwlock.suspend_list.remove({RWLockType::kRead, slave_cid_});
}

void Schedule::CoRWLockResume() {
  assert(is_master_);
  for (auto* rwlock : rwlocks_) {
    if (rwlock->lock_state == RWLockState::kWriteLock ||
        rwlock->lock_state == RWLockState::kReadLock) {
      continue; // 写锁或者读锁锁定，不需要唤醒其他从协程
    }
    if (rwlock->suspend_list.size() <= 0) continue;  // 锁已经释放了，但是没有挂起的从协程，也不需要唤醒
    // TODO，这里读锁和写锁的唤醒策略可以是不同的，后续再优化
    auto item = rwlock->suspend_list.front();
    rwlock->suspend_list.pop_front();
    CoroutineResume(item.second);  // 每次只能唤醒等待队列中的一个从协程，采用先进先出的策略
  }
}
} // namespace MyCoroutine