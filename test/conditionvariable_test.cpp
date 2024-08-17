#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mutex.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void CondNotifyOne(MyCoroutine::Schedule &schedule, MyCoroutine::CoCond &co_cond, list<int> &list) {
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
  }
  list.push_back(1);
  schedule.CoCondNotifyOne(co_cond);
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
  }
  list.push_back(2);
  schedule.CoCondNotifyOne(co_cond);
}

void CondWait1(MyCoroutine::Schedule &schedule, MyCoroutine::CoCond &co_cond, list<int> &list) {
  schedule.CoCondWait(co_cond, []() { return list.size() > 0; });
  assert(list.size() == 1);
  assert(list.front() == 1);
  list.pop_front();
}

void CondWait2(MyCoroutine::Schedule &schedule, MyCoroutine::CoCond &co_cond, list<int> &list) {
  schedule.CoCondWait(co_cond, []() { return list.size() > 0; });
  assert(list.size() == 1);
  assert(list.front() == 2);
  list.pop_front();
}
}  // namespace

// 协程条件变量测试用例1
TEST_CASE(CoCond_NotifyOne) {
  list<int> list;
  MyCoroutine::CoCond co_cond;
  MyCoroutine::Schedule schedule(1024);
  schedule.CoCondInit(co_cond);
  schedule.CoroutineCreate(CondNotifyOne, std::ref(schedule), std::ref(co_cond), std::ref(list));
  schedule.CoroutineCreate(CondWait1, std::ref(schedule), std::ref(co_cond), std::ref(list));
  schedule.CoroutineCreate(CondWait2, std::ref(schedule), std::ref(co_cond), std::ref(list));
  schedule.Run();
  schedule.CoCondClear(co_cond);
  ASSERT_EQ(list.size(), 0);
}
