#include <assert.h>

#include "mycoroutine.h"

namespace MyCoroutine {

int32_t Schedule::BatchCreate() {
  assert(not is_master_);
  for (int32_t i = 0; i < kMaxBatchSize; i++) {
    if (batchs_[i]->state == State::kIdle) {
      batchs_[i]->state = State::kReady;
      batchs_[i]->parent_cid = slave_cid_;
      coroutines_[slave_cid_]->bid = i;
      return i;
    }
  }
  return kInvalidBid;
}

void Schedule::BatchRun(int32_t bid) {
  // TODO
}
}  // namespace MyCoroutine

/*
int BatchInit(Schedule& schedule) {
  assert(not schedule.isMasterCoroutine);  // 从协程中才可以调用
  for (int i = 0; i < MAX_BATCH_RUN_SIZE; i++) {
    if (schedule.batchs[i]->state == Idle) {
      schedule.batchs[i]->state = Ready;
      schedule.batchs[i]->relateId = schedule.runningCoroutineId;
      schedule.coroutines[schedule.runningCoroutineId]->relateBatchId = i;
      schedule.coroutines[schedule.runningCoroutineId]->isInsertBatch = true;
      return i;
    }
  }
  return INVALID_BATCH_ID;
}

// void BatchAdd(Schedule& schedule, int batchId, Entry entry, void* arg, uint32_t priority) {
//   assert(not schedule.isMasterCoroutine);                                     // 从协程中才可以调用
//   assert(batchId >= 0 && batchId < MAX_BATCH_RUN_SIZE);                       // 校验batchId的合法性
//   assert(schedule.batchs[batchId]->state == Ready);                           // batch必须是ready的状态
//   assert(schedule.batchs[batchId]->relateId == schedule.runningCoroutineId);  // 关联的协程id必须正确
//   int id = CoroutineCreate(schedule, entry, arg, priority, batchId);
//   assert(id != INVALID_ROUTINE_ID);
//   schedule.batchs[batchId]->cid2finish[id] = false;  // 新增要执行的协程还没执行完
// }

void BatchRun(Schedule& schedule, int batchId) {
  assert(not schedule.isMasterCoroutine);                                     // 从协程中才可以调用
  assert(batchId >= 0 && batchId < MAX_BATCH_RUN_SIZE);                       // 校验batchId的合法性
  assert(schedule.batchs[batchId]->relateId == schedule.runningCoroutineId);  // 关联的协程id必须正确
  schedule.batchs[batchId]->state = Run;
  CoroutineYield(schedule);  // 这里的BatchRun只是一个卡点，等batch中所有的协程都执行完了，主协程再恢复从协程的执行
  schedule.batchs[batchId]->state = Idle;
  schedule.batchs[batchId]->cid2finish.clear();
  schedule.coroutines[schedule.runningCoroutineId]->relateBatchId = INVALID_BATCH_ID;
  schedule.coroutines[schedule.runningCoroutineId]->isInsertBatch = false;  // 重新设置未被插入batch卡点
}
*/