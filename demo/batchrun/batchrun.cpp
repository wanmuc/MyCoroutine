#include "mycoroutine.h"
#include "waitgroup.h"
#include <iostream>
using namespace std;

void BatchRunChild(int &sum) { sum++; }

void BatchRunParent(MyCoroutine::Schedule &schedule) {
  int sum = 0;
  MyCoroutine::WaitGroup wg(schedule);
  // 这里最多调用Add函数3次，最多添加3个批量的任务
  wg.Add(BatchRunChild, std::ref(sum));
  wg.Add(BatchRunChild, std::ref(sum));
  wg.Add(BatchRunChild, std::ref(sum));
  wg.Wait();
  cout << "sum = " << endl;
}

int main() {
  // 创建一个协程调度对象，生成大小为1024的协程池，每个协程中使用的Batch中最多添加3个批量任务
  MyCoroutine::Schedule schedule(1024, 3);
  schedule.CoroutineCreate(BatchRunParent, std::ref(schedule));
  schedule.Run(); // Run函数完成从协程的自行调度，直到所有的从协程都执行完
  return 0;
}