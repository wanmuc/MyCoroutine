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

void func3(int c) {
  cout << "func3 begin" << endl;
  cout << "func3.c=" << c << endl;
  schedule.CoroutineYield();
  cout << "func3 end" << endl;
}

int main() {
  schedule.CoroutineCreate(func1, 10);
  schedule.CoroutineCreate(func2, 20);
  schedule.CoroutineCreate(func3, 30);
  schedule.Run();
  return 0;
}
