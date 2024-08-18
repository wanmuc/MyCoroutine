#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "localvariable.h"
#include "mutex.h"
#include "mycoroutine.h"
#include "waitgroup.h"
using namespace std;

namespace {
void Sum(MyCoroutine::Schedule& schedule, int& total) {
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
    total++;
  }
}

void Sum2(int& total) { total += 10; }

void SumBatch(MyCoroutine::Schedule& schedule, int& total) {
  for (int i = 0; i < 10; i++) {
    schedule.CoroutineYield();
    total++;
  }
}

void SumHasBatch(MyCoroutine::Schedule& schedule, int& total) {
  int32_t bid = schedule.BatchCreate();
  schedule.BatchAdd(bid, SumBatch, std::ref(schedule), std::ref(total));
  schedule.BatchRun(bid);
}
}  // namespace

void BatchChild(MyCoroutine::Schedule& schedule, int& total) { total++; }

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

void WaitGroupSub(MyCoroutine::Schedule& schedule, int& total) { total++; }

void BatchWaitGroup(MyCoroutine::Schedule& schedule, int& total) {
  MyCoroutine::WaitGroup wait_group(schedule);
  wait_group.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group.Wait();

  MyCoroutine::WaitGroup wait_group2(schedule);
  wait_group2.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group2.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group2.Add(WaitGroupSub, std::ref(schedule), std::ref(total));
  wait_group2.Wait();
}

// 协程调度的测试用例1
TEST_CASE(Schedule_Run1) {
  int total = 0;
  MyCoroutine::Schedule schedule(1);
  for (int32_t i = 0; i < 1; i++) {
    int32_t cid = schedule.CoroutineCreate(Sum, std::ref(schedule), std::ref(total));
    ASSERT_EQ(cid, i);
  }
  schedule.Run();
  ASSERT_EQ(total, 10);
}

// 协程调度的测试用例2
TEST_CASE(Schedule_Run2) {
  int total = 0;
  MyCoroutine::Schedule schedule(10240);
  for (int32_t i = 0; i < 10240; i++) {
    int32_t cid = schedule.CoroutineCreate(Sum, std::ref(schedule), std::ref(total));
    ASSERT_EQ(cid, i);
  }
  schedule.Run();
  ASSERT_EQ(total, 102400);
  total = 0;
  for (int32_t i = 0; i < 10240; i++) {
    int32_t cid = schedule.CoroutineCreate(Sum2, std::ref(total));
    ASSERT_EQ(cid, i);
  }
  schedule.Run();
  ASSERT_EQ(total, 102400);
}

// 协程调度的测试用例3
TEST_CASE(Schedule_Run3) {
  int total = 0;
  MyCoroutine::Schedule schedule(5120, 1);
  for (int32_t i = 0; i < 5120; i++) {
    int32_t cid = schedule.CoroutineCreate(SumHasBatch, std::ref(schedule), std::ref(total));
    ASSERT_EQ(cid, i);
  }
  schedule.Run();
  ASSERT_EQ(total, 51200);
}

// 协程Batch特性的测试用例
TEST_CASE(Coroutine_Batch) {
  int total = 0;
  MyCoroutine::Schedule schedule(2560, 3);
  int32_t cid = schedule.CoroutineCreate(BatchParent, std::ref(schedule), std::ref(total));
  ASSERT_EQ(cid, 0);
  schedule.Run();
  ASSERT_EQ(total, 9);
}

// WaitGroup封装的测试用例
TEST_CASE(Coroutine_BatchWaitGroup) {
  int total = 0;
  MyCoroutine::Schedule schedule(2560, 3);
  int32_t cid = schedule.CoroutineCreate(BatchWaitGroup, std::ref(schedule), std::ref(total));
  ASSERT_EQ(cid, 0);
  schedule.Run();
  ASSERT_EQ(total, 6);
}

RUN_ALL_TESTS();
