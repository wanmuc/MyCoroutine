#include "mycoroutine.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
using namespace std;

void Sum(MyCoroutine::Schedule& schedule, int& total) {
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
    total++;
  }
}

void CoroutineLocalVariableFunc1(MyCoroutine::Schedule& schedule, MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
   local_variable.Set(100);
   schedule.CoroutineYield();
   assert(local_variable.Get() == 100);
}

void CoroutineLocalVariableFunc2(MyCoroutine::Schedule& schedule, MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
   local_variable.Set(200);
   schedule.CoroutineYield();
   assert(local_variable.Get() == 200);
}

void CoroutineLocalVariableFunc3(MyCoroutine::Schedule& schedule, MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
   local_variable.Set(300);
   schedule.CoroutineYield();
   assert(local_variable.Get() == 300);
}

void BatchChild(MyCoroutine::Schedule& schedule, int& total) {
  total++;
}

void BatchParent(MyCoroutine::Schedule& schedule, int& total) {
  int32_t bid = schedule.BatchCreate();
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchRun(bid);

  bid = schedule.BatchCreate();
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchRun(bid);

  bid = schedule.BatchCreate();
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchAdd(bid, BatchChild, std::ref(schedule), std::ref(total));
  schedule.BatchRun(bid);
}

void WaitGroupSub(MyCoroutine::Schedule& schedule, int& total) {
  total++;
}

void BatchWaitGroup(MyCoroutine::Schedule& schedule, int& total) {
  MyCoroutine::WaitGroup wait_group(&schedule);
  wait_group.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group.Wait();
}

// 协程调度的测试用例
TEST_CASE(Schedule_Run) {
  int total = 0;
  MyCoroutine::Schedule schedule(10240);
  for (int32_t i = 0; i < 10240; i++) {
    int32_t cid = schedule.CoroutineCreate(Sum, std::ref(schedule), std::ref(total));
    ASSERT_EQ(cid, i);
  }
  schedule.Run();
  ASSERT_EQ(total, 102400);
}

// 协程本地变量的测试用例
TEST_CASE(Coroutine_LocalVariable) {
  MyCoroutine::Schedule schedule(10240);
  MyCoroutine::CoroutineLocalVariable<int> local_variable(&schedule);
  schedule.CoroutineCreate(CoroutineLocalVariableFunc1, std::ref(schedule), std::ref(local_variable));
  schedule.CoroutineCreate(CoroutineLocalVariableFunc2, std::ref(schedule), std::ref(local_variable));
  schedule.CoroutineCreate(CoroutineLocalVariableFunc3, std::ref(schedule), std::ref(local_variable));
  schedule.Run();
}

// 协程Batch特性的测试用例
TEST_CASE(Coroutine_Batch) {
  int total = 0;
  MyCoroutine::Schedule schedule(10240);
  int32_t cid = schedule.CoroutineCreate(BatchParent, std::ref(schedule), std::ref(total));
  ASSERT_EQ(cid, 0);
  schedule.Run();
  ASSERT_EQ(total, 9);
}

// WaitGroup的测试用例
TEST_CASE(Coroutine_BatchWaitGroup) {
  int total = 0;
  MyCoroutine::Schedule schedule(10240);
  int32_t cid = schedule.CoroutineCreate(BatchWaitGroup, std::ref(schedule), std::ref(total));
  ASSERT_EQ(cid, 0);
  schedule.Run();
  ASSERT_EQ(total, 3);
}

RUN_ALL_TESTS();
