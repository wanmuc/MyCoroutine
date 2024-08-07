#include "schedule.h"

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

TEST_CASE(Schedule_Run) {
  int total = 0;
  MyCoroutine::Schedule schedule(10240);
  for (int32_t i = 0; i < 10240; i++) {
    int32_t cid = schedule.CoroutineCreate(Sum, std::ref(schedule), std::ref(total));
    ASSERT_EQ(cid, i);
  }
  schedule.Run();
  ASSERT_EQ(total, 204800);
}

RUN_ALL_TESTS();
