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
  // TODO
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
