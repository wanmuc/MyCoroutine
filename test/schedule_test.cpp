#include "schedule.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
using namespace std;

void Sum(MyCoroutine::Schedule& schedule, int & total) {
  total++;
  schedule.CoroutineYield();
  total++;
}

TEST_CASE(Schedule_Run) {
  int total = 0;
  MyCoroutine::Schedule schedule(10240);
  for (int32_t i = 0; i < 10240; i++) {
    int32_t cid = schedule.CoroutineCreate(Sum, std::ref(schedule), std::ref(total));
    ASSERT_EQ(cid, i);
  }
  ASSERT_EQ(total, 20480);
  schedule.Run();
}

RUN_ALL_TESTS();
