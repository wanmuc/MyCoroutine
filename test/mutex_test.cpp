#include "mutex.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void Mutex1(MyCoroutine::Schedule &schedule, MyCoroutine::CoMutex &co_mutex, int &value) {
  schedule.CoMutexLock(co_mutex);
  assert(value == 0);
  schedule.CoroutineYield();
  value++;
  schedule.CoMutexUnLock(co_mutex);
}
void Mutex2(MyCoroutine::Schedule &schedule, MyCoroutine::CoMutex &co_mutex, int &value) {
  schedule.CoMutexLock(co_mutex);
  assert(value == 1);
  schedule.CoroutineYield();
  value++;
  schedule.CoMutexUnLock(co_mutex);
}
void Mutex3(MyCoroutine::Schedule &schedule, MyCoroutine::CoMutex &co_mutex, int &value) {
  schedule.CoMutexLock(co_mutex);
  assert(value == 2);
  schedule.CoroutineYield();
  value++;
  schedule.CoMutexUnLock(co_mutex);
}
}  // namespace

// 协程互斥量
TEST_CASE(CoMutex_LockAndUnLock) {
  int value = 0;
  MyCoroutine::CoMutex co_mutex;
  MyCoroutine::Schedule schedule(1024);
  schedule.CoMutexInit(co_mutex);
  schedule.CoroutineCreate(Mutex1, std::ref(schedule), std::ref(CoMutex), std::ref(value));
  schedule.CoroutineCreate(Mutex2, std::ref(schedule), std::ref(CoMutex), std::ref(value));
  schedule.CoroutineCreate(Mutex3, std::ref(schedule), std::ref(CoMutex), std::ref(value));
  schedule.Run();
  schedule.CoMutexClear(co_mutex);
  ASSERT_EQ(value, 3);
}
