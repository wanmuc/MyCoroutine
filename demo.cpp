#include <iostream>

#include "schedule.h"

using namespace std;

MyCoroutine::Schedule schedule(1024);

void func1(int a) {
  cout << "func1 begin" << endl;
  cout << "func1.a=" << a << endl;
  schedule.CoroutineYield();
  cout << "func1 end" << endl;
}

void func2(int b) {
  cout << "func2 begin" << endl;
  cout << "func2.b=" << b << endl;
  schedule.CoroutineYield();
  cout << "func2 end" << endl;
}

int main() {
  schedule.CoroutineCreate(fun1, 10);
  schedule.CoroutineCreate(fun2, 20);
  schedule.Run();
  return 0;
}
