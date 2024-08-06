#pragma once

#include "coroutine.h"

namespace MyCoroutine {

constexpr int32_t kStackSize = 64 * 1024;     // 协程栈默认大小为 64K
constexpr int32_t kInvalidCid = -1;           // 无效的协程 id
constexpr int32_t kMaxCoroutineSize = 10240;  // 允许创建的最大协程池大小

// 协程调度器
class Schedule {
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
  static void CoroutineRun(Schedule* schedule);

 private:
  bool is_master_{true};                      // 是否主协程
  ucontext_t main_;                           // 保存主协程的上下文
  int32_t slave_cid_{kInvalidCid};            // 运行中的从协程的 id（运行从协程时才有效）
  int32_t total_count_{0};                    // 从协程总数
  int32_t suspend_and_run_count_{0};          // 运行中和挂起的从协程数
  int32_t stack_size_{kStackSize};            // 从协程栈大小，单位字节
  Coroutine* coroutines_[kMaxCoroutineSize];  // 从协程数组池
};
}  // namespace MyCoroutine