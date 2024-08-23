#include "coroutine.h"
#include <iostream>
using namespace std;

void HelloWorld(MyCoroutine::Schedule & schedule) {
  cout << "hello ";
  schedule.CoroutineYield();
  cout << "world" << endl;
}

int main() {
  MyCoroutine::Schedule schedule(1024);
  int32_t cid = schedule.CoroutineCreate(HelloWorld, std::ref(schedule));
  schedule.CoroutineResume(cid);
  schedule.CoroutineResume(cid);
  return 0;
}