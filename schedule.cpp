#include "schedule.h"

namespace MyCoroutine {

void Schedule::CoroutineRun() {
  is_master_ = false;
  suspend_and_run_count_++;
  Coroutine* routine = coroutines_[slave_cid_];
  routine->Entry();
  slave_cid_ = kInvalidCid;  // slave_cid_更新为无效的从协程id
  suspend_and_run_count_--;
  // 函数执行完，调用栈会回到主协程，执行routine->ctx_.uc_link指向的上下文的下一条指令
}

Schedule::Schedule(int32_t total_count) : total_count_(total_count) {
  assert(total_count_ > 0 && total_count_ <= kMaxCoroutineSize);
  for (int32_t i = 0; i < total_count_; i++) {
    coroutines_[i] = new Coroutine;
  }
}

Schedule::~Schedule() {
  for (int32_t i = 0; i < total_count_; i++) {
    if (coroutines_[i]->stack_) {
      delete[] coroutines_[i]->stack_;
    }
    delete coroutines_[i];
  }
}

void Schedule::Run() {
  while (suspend_and_run_count_ > 0) {
  }
}

void Schedule::CoroutineInit(Coroutine* routine, std::function<void()> entry) {
  routine->entry_ = entry;
  routine->state_ = kReady;
  routine->stack_ = new uint8_t[stack_size_];
  getcontext(&(routine->ctx_));
  routine->ctx_.uc_stack.ss_flags = 0;
  routine->ctx_.uc_stack.ss_sp = routine->stack;
  routine->ctx_.uc_stack.ss_size = schedule.stackSize;
  routine->ctx_.uc_link = &(main_);
  // 设置routine->ctx_上下文要执行的函数和对应的参数，
  // 这里没有直接使用entry，而是多包了一层CoroutineRun函数的调用，
  // 是为了在CoroutineRun中entry函数执行完之后，从协程的状态更新kIdle，并更新当前处于运行中的从协程id为无效id，
  // 这样这些逻辑就可以对上层调用透明。
  makecontext(&(routine->ctx_), (void (*)(void))(CoroutineRun), 1, this);
}

/*

bool ScheduleRunning(Schedule& schedule) {
  assert(schedule.isMasterCoroutine);
  if (schedule.runningCoroutineId != kInvalidRoutineId) return true;
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    if (schedule.coroutines[i]->state != Idle) return true;
  }
  return false;
}

 */

}  // namespace MyCoroutine

/*
 * class Schedule {
 public:
  explicit Schedule(int32_t total_count);
  ~Schedule();
  void Run();

  template <typename Function, typename... Args>
  int32_t CoroutineCreate(Function&& func, Args&&... args) {
    int32_t cid = 0;
    for (cid = 0; cid < total_count_; cid++) {
      if (coroutines_[cid]->state_ == State::kIdle) {
        break;
      }
    }
    if (cid >= total_count_) {
      return kInvalidCid;
    }
    Coroutine* routine = coroutines[cid];
    std::function<void()> entry = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);
    CoroutineInit(routine, entry);
    return cid;
  }
  void CoroutineYield();
  void CoroutineResume(int32_t cid);

 private:
  void CoroutineInit(Coroutine* routine, std::function<void()> entry);
 */
/*
 * // 协程调度结构体初始化
int ScheduleInit(Schedule& schedule, int coroutineCnt, int stackSize = 8 * 1024);
// 判断是否还有协程在运行
bool ScheduleRunning(Schedule& schedule);
// 调度器主动驱动协程执行
void ScheduleRun(Schedule& schedule);
// 释放调度器
void ScheduleClean(Schedule& schedule);
 *
 */