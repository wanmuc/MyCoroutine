#include <assert.h>

#include "mycoroutine.h"

namespace MyCoroutine {

int32_t Schedule::BatchCreate() {
  assert(not is_master_);
  for (int32_t i = 0; i < kMaxBatchSize; i++) {
    if (batchs_[i]->state == State::kIdle) {
      batchs_[i]->state = State::kReady;
      batchs_[i]->parent_cid = slave_cid_;  // 设置批量执行关联的父从协程
      coroutines_[slave_cid_]->bid = i;     // 设置从协程关联的批量执行
      return i;
    }
  }
  return kInvalidBid;
}

void Schedule::BatchRun(int32_t bid) {
  assert(not is_master_);
  assert(bid >= 0 && bid < kMaxBatchSize);
  assert(batchs_[bid]->parent_cid == slave_cid_);
  batchs_[bid]->state = State::kRun;
  CoroutineYield();  // BatchRun只是一个卡点，等batch中所有的子从协程都执行完了，主协程再恢复父从协程的执行
  batchs_[bid]->state = State::kIdle;
  batchs_[bid]->parent_cid = kInvalidCid;
  batchs_[bid]->child_cid_2_finish.clear();
  coroutines_[slave_cid_]->bid = kInvalidBid;
}
}  // namespace MyCoroutine
