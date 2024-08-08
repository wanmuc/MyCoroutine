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

TEST_CASE(CoroutineLocalVariable) {
  MyCoroutine::Schedule schedule(10240);
  MyCoroutine::CoroutineLocalVariable<int> local_variable;
  schedule.CoroutineCreate(CoroutineLocalVariableFunc1, std::ref(schedule), std::ref(local_variable));
  schedule.CoroutineCreate(CoroutineLocalVariableFunc2, std::ref(schedule), std::ref(local_variable));
  schedule.CoroutineCreate(CoroutineLocalVariableFunc3, std::ref(schedule), std::ref(local_variable));
  schedule.Run();
}

RUN_ALL_TESTS();
