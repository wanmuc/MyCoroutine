#include "rwlock.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void WrLockWrap1(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::WrLockGuard lock_guard(rwlock);
  assert(value == 4);
  value++;
  schedule.CoroutineYield();
}
void WrLockWrap2(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::WrLockGuard lock_guard(rwlock);
  assert(value == 5);
  value++;
  schedule.CoroutineYield();
}
void WrLockWrap3(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::WrLockGuard lock_guard(rwlock);
  assert(value == 3);
  value++;
  schedule.CoroutineYield();
}
void RdLockWrap1(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::RdLockGuard lock_guard(rwlock);
  assert(value == 0);
  value++;
  schedule.CoroutineYield();
}
void RdLockWrap2(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::RdLockGuard lock_guard(rwlock);
  assert(value == 1);
  value++;
  schedule.CoroutineYield();
}
void RdLockWrap3(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::RdLockGuard lock_guard(rwlock);
  assert(value == 2);
  value++;
  schedule.CoroutineYield();
}
void WrLockResume(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value) {
  MyCoroutine::WrLockGuard lock_guard(rwlock);
  assert(value == 0);
  value++;
  schedule.CoroutineYield();
  value++;
}
void RdLockResume(MyCoroutine::Schedule &schedule, MyCoroutine::RWLock &rwlock, int &value, int &rvalue) {
  MyCoroutine::RdLockGuard lock_guard(rwlock);
  assert(value == 2);
  rvalue++;
}
}  // namespace

// 协程读写锁测试用例-WrLockAndRdLockWrap
TEST_CASE(CoRWLock_WrLockAndRdLockWrap) {
  int value = 0;
  MyCoroutine::Schedule schedule(1024);
  MyCoroutine::RWLock rwlock(schedule);
  schedule.CoroutineCreate(RdLockWrap1, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.CoroutineCreate(RdLockWrap2, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.CoroutineCreate(WrLockWrap1, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.CoroutineCreate(WrLockWrap2, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.CoroutineCreate(RdLockWrap3, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.CoroutineCreate(WrLockWrap3, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.Run();
  ASSERT_EQ(value, 6);
}

// 协程读写锁测试用例-Resume-读锁因为写锁而挂起，后面被唤醒
TEST_CASE(CoRWLock_Resume_WrLockThenRdLock) {
  int value = 0;
  int rvalue = 0;
  MyCoroutine::Schedule schedule(1024);
  MyCoroutine::RWLock rwlock(schedule);
  schedule.CoroutineCreate(WrLockResume, std::ref(schedule), std::ref(rwlock), std::ref(value));
  schedule.CoroutineCreate(RdLockResume, std::ref(schedule), std::ref(rwlock), std::ref(value), std::ref(rvalue));
  schedule.CoroutineCreate(RdLockResume, std::ref(schedule), std::ref(rwlock), std::ref(value), std::ref(rvalue));
  schedule.CoroutineResume(0);  // 第一次加写锁成功
  ASSERT_EQ(value, 1);
  schedule.CoroutineResume(1);  // 读锁被挂起
  schedule.CoroutineResume(2);  // 读锁被挂起
  ASSERT_EQ(rvalue, 0);
  schedule.CoroutineResume(0);  // 写锁释放
  ASSERT_EQ(value, 2);
  schedule.CoRWLockResume();  // 唤醒所有的读锁
  ASSERT_EQ(rvalue, 2);
}