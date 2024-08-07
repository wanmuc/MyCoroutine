#pragma once

#include "coroutine.h"

namespace MyCoroutine {
constexpr int32_t kStackSize = 64 * 1024;     // 协程栈默认大小为 64K
constexpr int32_t kMaxCoroutineSize = 10240;  // 允许创建的最大协程池大小

// 协程调度器
class Schedule {
 public:
  explicit Schedule(int32_t total_count);
  ~Schedule();
  void Run();
  // 从协程的创建函数，通过模版函数，可以支持不同原型的函数，作为从协程的执行函数
  template <typename Function, typename... Args>
  int32_t CoroutineCreate(Function &&func, Args &&...args) {
    int32_t cid = 0;
    for (cid = 0; cid < total_count_; cid++) {
      if (coroutines_[cid]->state == State::kIdle) {
        break;
      }
    }
    if (cid >= total_count_) {
      return kInvalidCid;
    }
    Coroutine *routine = coroutines_[cid];
    std::function<void()> entry = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);
    CoroutineInit(routine, entry);
    return cid;
  }
  // 从协程让出cpu执行权
  void CoroutineYield();
  // 主协程唤醒指定的从协程
  void CoroutineResume(int32_t cid);

 private:
  // 从协程的执行入口
  static void CoroutineRun(Schedule *schedule, Coroutine* routine);
  // 从协程的初始化
  void CoroutineInit(Coroutine *routine, std::function<void()> entry);

 private:
  ucontext_t main_;                           // 保存主协程的上下文
  bool is_master_{true};                      // 是否主协程
  int32_t slave_cid_{kInvalidCid};            // 运行中的从协程的id（运行从协程时才有效）
  int32_t not_idle_count_{0};                 // 运行中和挂起的从协程数
  int32_t total_count_{0};                    // 从协程总数
  int32_t stack_size_{kStackSize};            // 从协程栈大小，单位字节
  Coroutine *coroutines_[kMaxCoroutineSize];  // 从协程数组池
};
}  // namespace MyCoroutine