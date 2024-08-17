#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mutex.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void CondNotifyOne(MyCoroutine::Schedule &schedule, MyCoroutine::CoCond &co_cond, list<int> &queue) {
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
  }
  queue.push_back(1);
  schedule.CoCondNotifyOne(co_cond);
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
  }
  queue.push_back(2);
  schedule.CoCondNotifyOne(co_cond);
}

void CondWait1(MyCoroutine::Schedule &schedule, MyCoroutine::CoCond &co_cond, list<int> &queue) {
  schedule.CoCondWait(co_cond, []() { return queue.size() > 0; });
  assert(queue.size() == 1);
  assert(queue.front() == 1);
  queue.pop_front();
}

void CondWait2(MyCoroutine::Schedule &schedule, MyCoroutine::CoCond &co_cond, list<int> &queue) {
  schedule.CoCondWait(co_cond, []() { return queue.size() > 0; });
  assert(queue.size() == 1);
  assert(queue.front() == 2);
  queue.pop_front();
}
}  // namespace

// 协程条件变量测试用例1
TEST_CASE(CoCond_NotifyOne) {
  list<int> queue;
  MyCoroutine::CoCond co_cond;
  MyCoroutine::Schedule schedule(1024);
  schedule.CoCondInit(co_cond);
  schedule.CoroutineCreate(CondNotifyOne, std::ref(schedule), std::ref(co_cond), std::ref(queue));
  schedule.CoroutineCreate(CondWait1, std::ref(schedule), std::ref(co_cond), std::ref(queue));
  schedule.CoroutineCreate(CondWait2, std::ref(schedule), std::ref(co_cond), std::ref(queue));
  schedule.Run();
  schedule.CoCondClear(co_cond);
  ASSERT_EQ(queue.size(), 0);
}
