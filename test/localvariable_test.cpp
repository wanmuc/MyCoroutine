#include "localvariable.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mycoroutine.h"
using namespace std;

namespace {
void CoroutineLocalVariableFunc1(MyCoroutine::Schedule& schedule,
                                 MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
  local_variable.Set(100);
  schedule.CoroutineYield();
  assert(local_variable.Get() == 100);
}

void CoroutineLocalVariableFunc2(MyCoroutine::Schedule& schedule,
                                 MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
  local_variable.Set(200);
  schedule.CoroutineYield();
  assert(local_variable.Get() == 200);
}

void CoroutineLocalVariableFunc3(MyCoroutine::Schedule& schedule,
                                 MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
  local_variable.Set(300);
  schedule.CoroutineYield();
  assert(local_variable.Get() == 300);
}
}  // namespace

// 协程本地变量的测试用例
TEST_CASE(Coroutine_LocalVariable) {
  MyCoroutine::Schedule schedule(10240);
  MyCoroutine::CoroutineLocalVariable<int> local_variable(schedule);
  schedule.CoroutineCreate(CoroutineLocalVariableFunc1, std::ref(schedule), std::ref(local_variable));
  schedule.CoroutineCreate(CoroutineLocalVariableFunc2, std::ref(schedule), std::ref(local_variable));
  schedule.CoroutineCreate(CoroutineLocalVariableFunc3, std::ref(schedule), std::ref(local_variable));
  schedule.Run();
}
