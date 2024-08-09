#include <assert.h>

#include "mycoroutine.h"

namespace MyCoroutine {

void Schedule::CoroutineRun(Schedule* schedule, Coroutine* routine) {
  assert(not schedule->is_master_);
  schedule->is_master_ = false;
  schedule->slave_cid_ = routine->cid;
  routine->entry();  // 执行从协程的入口函数
  assert(routine->state == State::kRun);
  routine->state = State::kIdle;
  schedule->is_master_ = true;
  schedule->slave_cid_ = kInvalidCid;  // slave_cid_更新为无效的从协程id
  schedule->not_idle_count_--;
  int32_t cid = routine->cid;
  int32_t bid = routine->relate_bid;
  // 从协程有关联Batch，且是Batch中的子从协程
  if (bid != kInvalidBid && routine->cid != schedule->batchs_[bid]->parent_cid) {
    assert(schedule->batchs_[bid]->child_cid_2_finish.find(cid) !=
           schedule->batchs_[bid]->child_cid_2_finish.end());
    schedule->batchs_[bid]->child_cid_2_finish[cid] = true;
    if (schedule->IsBatchDone(bid)) {
      schedule->batch_finish_cid_list.push_back(cid);
    }
    routine->relate_bid = kInvalidBid;
  }
  // CoroutineRun执行完，调用栈会回到主协程，执行routine->ctx.uc_link指向的上下文的下一条指令
  // 即CoroutineResume函数中的swapcontext调用返回了。
}

Schedule::Schedule(int32_t total_count) : total_count_(total_count) {
  assert(total_count_ > 0 && total_count_ <= kMaxCoroutineSize);
  for (int32_t i = 0; i < total_count_; i++) {
    coroutines_[i] = new Coroutine;
    coroutines_[i]->cid = i;
  }
  for (int32_t i = 0; i < kMaxBatchSize; i++) {
    batchs_[i] = new Batch;
    batchs_[i]->bid = i;
  }
}

Schedule::~Schedule() {
  for (int32_t i = 0; i < total_count_; i++) {
    if (coroutines_[i]->stack) delete[] coroutines_[i]->stack;
    for (const auto& item : coroutines_[i]->local) {
      item.second.free(item.second.data);  // 释放协程本地变量的内存
    }
    delete coroutines_[i];
  }
}

void Schedule::Run() {
  assert(is_master_);
  while (not_idle_count_ > 0) {
    for (int32_t i = 0; i < total_count_; i++) {
      if (coroutines_[i]->state == State::Idle || coroutines_[i]->state == State::kRun) {
        continue;
      }
      // 从协程中没有Batch的，直接唤醒从协程的执行
      if (coroutines_[i]->relate_bid == kInvalidBid) {
        CoroutineResume(i);
        continue;
      }
      // 从协程中有Batch的，要判断是Batch是否已经执行完
      if (batch_finish_cid_list.find(i) != batch_finish_cid_list.end()) {
        CoroutineResume(i);
        batch_finish_cid_list.erase(i);  // 唤醒之后，需要立即从batch_finish_cid_list删除对应的cid
      }
    }
  }
}

void Schedule::CoroutineYield() {
  assert(not is_master_);
  assert(slave_cid_ >= 0 && slave_cid_ < total_count_);
  Coroutine* routine = coroutines_[slave_cid_];
  assert(routine->state == State::kRun);
  routine->state = State::kSuspend;
  is_master_ = true;
  slave_cid_ = kInvalidCid;
  swapcontext(&routine->ctx, &main_);
  is_master_ = false;
  slave_cid_ = routine->cid;
}

void Schedule::CoroutineResume(int32_t cid) {
  assert(is_master_);
  assert(cid >= 0 && cid < total_count_);
  Coroutine* routine = coroutines_[cid];
  assert(coroutines_[cid]->state == State::kReady || coroutines_[cid]->state == State::kSuspend);
  if (routine->relate_bid != kInvalidBid) {
    assert(batch_finish_cid_list.find(cid) != batch_finish_cid_list.end());
  }
  routine->state = State::kRun;
  is_master_ = false;
  slave_cid_ = cid;
  // 切换到协程编号为cid的从协程中执行，并把当前执行上下文保存到main_中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&main_, &routine->ctx);
  is_master_ = true;
  slave_cid_ = kInvalidCid;
}

void Schedule::CoroutineInit(Coroutine* routine, std::function<void()> entry) {
  routine->entry = entry;
  routine->state = State::kReady;
  if (nullptr == routine->stack) {
    routine->stack = new uint8_t[stack_size_];
  }
  getcontext(&routine->ctx);
  routine->ctx.uc_stack.ss_flags = 0;
  routine->ctx.uc_stack.ss_sp = routine->stack;
  routine->ctx.uc_stack.ss_size = stack_size_;
  routine->ctx.uc_link = &main_;
  not_idle_count_++;
  // 设置routine->ctx上下文要执行的函数和对应的参数，
  // 这里没有直接使用entry，而是多包了一层CoroutineRun函数的调用，
  // 是为了在CoroutineRun中entry函数执行完之后，从协程的状态更新kIdle，并更新当前处于运行中的从协程id为无效id，
  // 这样这些逻辑就可以对上层调用透明。
  makecontext(&(routine->ctx), (void (*)(void))(CoroutineRun), 2, this, routine);
}
}  // namespace MyCoroutine