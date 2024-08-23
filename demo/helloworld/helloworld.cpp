#include "mycoroutine.h"
#include <iostream>
using namespace std;

void HelloWorld(MyCoroutine::Schedule &schedule) {
  cout << "hello ";
  schedule.CoroutineYield();
  cout << "world" << endl;
}

int main() {
  MyCoroutine::Schedule schedule(1024);
  // 创建一个从协程，并手动调度
  {
    int32_t cid = schedule.CoroutineCreate(HelloWorld, std::ref(schedule));
    schedule.CoroutineResume(cid);
    schedule.CoroutineResume(cid);
  }
  // 创建一个从协程，并自行调度
  {
    schedule.CoroutineCreate(HelloWorld, std::ref(schedule));
    schedule.Run(); // 协程库，自行调度协程的执行
  }
  return 0;
}