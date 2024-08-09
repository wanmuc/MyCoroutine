#include <assert.h>

#include "mycoroutine.h"

namespace MyCoroutine {

int32_t Schedule::BatchCreate() {
  assert(not is_master_);
  for (int32_t i = 0; i < kMaxBatchSize; i++) {
    if (batchs_[i]->state == State::kIdle) {
      batchs_[i]->state = State::kReady;
      batchs_[i]->parent_cid = slave_cid_;      // 设置批量执行关联的父从协程
      coroutines_[slave_cid_]->relate_bid = i;  // 设置从协程关联的批量执行
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
  CoroutineYield();  // BatchRun只是一个卡点，等Batch中所有的子从协程都执行完了，主协程再恢复父从协程的执行
  batchs_[bid]->state = State::kIdle;
  batchs_[bid]->parent_cid = kInvalidCid;
  batchs_[bid]->child_cid_2_finish.clear();
  coroutines_[slave_cid_]->relate_bid = kInvalidBid;
}

void Schedule::CoroutineResume4BatchStart(int32_t cid) {
  assert(is_master_);
  assert(cid >= 0 && cid < total_count_);
  Coroutine * routine = coroutines_[cid];
  // 从协程中没有关联的Batch，则没有需要唤醒的子从协程
  if (routine->relate_bid == kInvalidBid) {
    return;
  }
  int32_t bid = routine->relate_bid;
  for (const auto & item : batchs_[bid]->child_cid_2_finish) {
    CoroutineResume(item->first);  // 唤醒Batch中的子从协程
  }
}

void Schedule::CoroutineResume4BatchFinish() {
  assert(is_master_);
  assert(batch_finish_list.size() <= 0);
  for (const auto & cid : batch_finish_list) {
    CoroutineResume(cid);
  }
  batch_finish_list.clear();
}
}  // namespace MyCoroutine
