#include "semaphore.h"

#include <assert.h>

#include <iostream>
#include <queue>

#include "UTestCore.h"
#include "mycoroutine.h"
using namespace std;

namespace {

void Producer(MyCoroutine::Schedule &schedule, MyCoroutine::Semaphore &sem, queue<int> &q) {
  for (int i = 1; i <= 100; i++) {
    sem.Post();
    q.push(i);
    schedule.CoroutineYield();
  }
}

void Consumer(MyCoroutine::Schedule &schedule, MyCoroutine::Semaphore &sem, queue<int> &q, int &value) {
  while (value != 10100) {
    sem.Wait();
    assert(not q.empty());
    value += q.front();
    q.pop();
    schedule.CoroutineYield();
  }
}
}  // namespace

// 协程Semaphore测试用例
TEST_CASE(CoSemaphore_ALL) {
  int value = 0;
  queue<int> q;
  MyCoroutine::Schedule schedule(1024);
  MyCoroutine::Semaphore sem(schedule, 0);
  schedule.CoroutineCreate(Producer, std::ref(schedule), std::ref(sem), std::ref(q));
  schedule.CoroutineCreate(Producer, std::ref(schedule), std::ref(sem), std::ref(q));
  schedule.CoroutineCreate(Consumer, std::ref(schedule), std::ref(sem), std::ref(q), std::ref(value));
  schedule.CoroutineCreate(Consumer, std::ref(schedule), std::ref(sem), std::ref(q), std::ref(value));
  schedule.Run();
  ASSERT_EQ(q.size(), 0);
  ASSERT_EQ(value, 10100);
}
