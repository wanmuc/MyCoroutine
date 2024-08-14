#include "mycoroutine.h"

namespace MyCoroutine {
void Schedule::MutexInit(Mutex& mutex) {
  mutex.id = schedule.mutexManage.alloc_id;
  mutex.lock = false;
  mutex.cid = schedule.runningCoroutineId;
  schedule.mutexManage.alloc_id++;
  assert(schedule.mutexManage.mutexs.find(mutex.id) == schedule.mutexManage.mutexs.end());
  schedule.mutexManage.mutexs[mutex.id] = &mutex;
}

void CoMutexClear(Schedule& schedule, CoMutex& mutex) { schedule.mutexManage.mutexs.erase(mutex.id); }

void CoMutexLock(Schedule& schedule, CoMutex& mutex) {
  while (true) {
    assert(not schedule.isMasterCoroutine);
    if (not mutex.lock) {
      mutex.lock = true;  // 加锁成功，直接返回
      mutex.cid = schedule.runningCoroutineId;
      return;
    }
    // 不可重入，同一个从协程自能锁定一次，不能锁定多次
    assert(mutex.cid != schedule.runningCoroutineId);
    // 更新因为等待互斥量而被挂起的从协程id
    if (mutex.suspend_id_set.find(schedule.runningCoroutineId) == mutex.suspend_id_set.end()) {
      mutex.suspend_id_set.insert(schedule.runningCoroutineId);
      mutex.suspend_id_list.push_back(schedule.runningCoroutineId);
    }
    // 从协程让出执行权
    CoroutineYield(schedule);
  }
}

void CoMutexUnLock(Schedule& schedule, CoMutex& mutex) {
  assert(not schedule.isMasterCoroutine);
  assert(mutex.lock);  // 必须是锁定的
  mutex.lock = false;  // 设置层false即可，后续由调度器schedule去激活那些被挂起的从协程
  mutex.cid = kInvalidRoutineId;
}
}  // namespace MyCoroutine
