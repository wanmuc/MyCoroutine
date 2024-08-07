#include "schedule.h"

#include <assert.h>

namespace MyCoroutine {

void Schedule::CoroutineRun(Schedule* schedule, Coroutine* routine) {
  assert(schedule->is_master_);
  schedule->is_master_ = false;
  schedule->slave_cid_ = routine->cid;
  routine->entry();  // 执行从协程的入口函数
  assert(routine->state == State::kRun);
  routine->state = State::kIdle;
  schedule->is_master_ = true;
  schedule->slave_cid_ = kInvalidCid;  // slave_cid_更新为无效的从协程id
  schedule->not_idle_count_--;
  // CoroutineRun执行完，调用栈会回到主协程，执行routine->ctx.uc_link指向的上下文的下一条指令
  // 即CoroutineResume函数中的swapcontext调用返回了。
}

Schedule::Schedule(int32_t total_count) : total_count_(total_count) {
  assert(total_count_ > 0 && total_count_ <= kMaxCoroutineSize);
  for (int32_t i = 0; i < total_count_; i++) {
    coroutines_[i] = new Coroutine;
    coroutines_[i]->cid = i;
  }
}

Schedule::~Schedule() {
  for (int32_t i = 0; i < total_count_; i++) {
    if (coroutines_[i]->stack) delete[] coroutines_[i]->stack;
    delete coroutines_[i];
  }
}

void Schedule::Run() {
  while (not_idle_count_ > 0) {
    for (int32_t i = 0; i < total_count_; i++) {
      if (coroutines_[i]->state == State::kReady || coroutines_[i]->state == State::kSuspend) {
        CoroutineResume(i);
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
  Coroutine *routine = coroutines_[cid];
  assert(coroutines_[cid]->state == State::kReady || coroutines_[cid]->state == State::kSuspend);
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