#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mutex.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void BatchChild(int &value) { value++; }
void BatchParent(MyCoroutine::Schedule &schedule, int &value) {
  int bid = schedule.BatchCreate();
  schedule.BatchAdd(bid, BatchChild, std::ref(value));
  schedule.BatchAdd(bid, BatchChild, std::ref(value));
  bool add = schedule.BatchAdd(bid, BatchChild, std::ref(value));
  assert(add == false);
  schedule.BatchRun(bid);
}
}  // namespace

// Batch测试用例-All
TEST_CASE(Batch_All) {
  int value = 0;
  MyCoroutine::Schedule schedule(1024, 2);
  schedule.CoroutineCreate(BatchParent, std::ref(schedule), std::ref(value));
  schedule.CoroutineResume(0);
  schedule.CoroutineResume4BatchStart();
  schedule.CoroutineResume4BatchFinish();
  ASSERT_EQ(value, 2);
}
