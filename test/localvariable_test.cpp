#include "localvariable.h"

#include <assert.h>

#include <iostream>

#include "UTestCore.h"
#include "mycoroutine.h"
#include "waitgroup.h"

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

void CoroutineLocalVariableWithBatchChild(MyCoroutine::Schedule& schedule,
                                          MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
  assert(local_variable.Get() == 100);
}

void CoroutineLocalVariableWithBatch(MyCoroutine::Schedule& schedule,
                                     MyCoroutine::CoroutineLocalVariable<int>& local_variable) {
  MyCoroutine::WaitGroup wg(schedule);
  local_variable.Set(100);
  wg.Add(CoroutineLocalVariableWithBatchChild, std::ref(schedule), std::ref(local_variable));
  wg.Add(CoroutineLocalVariableWithBatchChild, std::ref(schedule), std::ref(local_variable));
  wg.Wait();
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

// 协程本地变量的测试用例
TEST_CASE(Coroutine_LocalVariableWithBatch) {
  MyCoroutine::Schedule schedule(1024, 2);
  MyCoroutine::CoroutineLocalVariable<int> local_variable(schedule);
  schedule.CoroutineCreate(CoroutineLocalVariableWithBatch, std::ref(schedule), std::ref(local_variable));
  schedule.Run();
}
