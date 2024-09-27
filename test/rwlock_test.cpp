#include "rwlock.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void WrLockWrap(MyCoroutine::Schedule &schedule, MyCoroutine::CoMutex &co_rwlock, int &value) {
  MyCoroutine::WrLockGuard lock_guard(co_rwlock);
  assert(value == 3);
  value++;
  schedule.CoroutineYield();
}
void RdLockWrap1(MyCoroutine::Schedule &schedule, MyCoroutine::CoRWLock &co_rwlock, int &value) {
  MyCoroutine::RdLockGuard lock_guard(co_rwlock);
  assert(value == 0);
  value++;
  schedule.CoroutineYield();
}
void RdLockWrap2(MyCoroutine::Schedule &schedule, MyCoroutine::CoRWLock &co_rwlock, int &value) {
  MyCoroutine::RdLockGuard lock_guard(co_rwlock);
  assert(value == 1);
  value++;
  schedule.CoroutineYield();
}
void RdLockWrap3(MyCoroutine::Schedule &schedule, MyCoroutine::CoRWLock &co_rwlock, int &value) {
  MyCoroutine::RdLockGuard lock_guard(co_rwlock);
  assert(value == 2);
  value++;
  schedule.CoroutineYield();
}
}  // namespace

// 协程读写锁测试用例-WrLockAndRdLock
TEST_CASE(CoRWLock_WrLockAndRdLockWrap) {
  int value = 0;
  MyCoroutine::CoRWLock co_rwlock;
  MyCoroutine::Schedule schedule(1024);
  schedule.CoRWLockInit(co_rwlock);
  schedule.CoroutineCreate(RdLockWrap1, std::ref(schedule), std::ref(co_rwlock), std::ref(value));
  schedule.CoroutineCreate(RdLockWrap2, std::ref(schedule), std::ref(co_rwlock), std::ref(value));
  schedule.CoroutineCreate(RdLockWrap3, std::ref(schedule), std::ref(co_rwlock), std::ref(value));
  schedule.CoroutineCreate(WrLockWrap, std::ref(schedule), std::ref(co_rwlock), std::ref(value));
  schedule.Run();
  schedule.CoRWLockClear(co_rwlock);
  ASSERT_EQ(value, 4);
}
