#pragma once

#include "common.h"
#include <assert.h>

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

  // 设置协程本地变量
  void LocalVariableSet(void *key, const LocalVariable &local_variable);
  // 获取协程本地变量
  bool LocalVariableGet(void *key, LocalVariable &local_variable);

 private:
  // 从协程的执行入口
  static void CoroutineRun(Schedule *schedule, Coroutine *routine);
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

// 协程本地变量模版类封装
template <typename Type>
class CoroutineLocalVariable {
 public:
  CoroutineLocalVariable(Schedule * schedule) {
    schedule_ = schedule;
  }
  static void Free(void* data) {
    if (data) delete (Type*)data;
  }
  void Set(Type value) {
    Type* temp = new Type(value);
    MyCoroutine::LocalVariable local_variable{temp, Free};
    // local_variable.data = temp;
    // local_variable.free = Free;
    schedule_->LocalVariableSet(this, local_variable);
  }
  Type& Get() {
    MyCoroutine::LocalVariable local_variable;
    bool result = schedule_->LocalVariableGet(this, local_variable);
    assert(result == true);
    return *(Type*)local_variable.data;
  }
 private:
  Schedule * schedule_{nullptr};
};
}  // namespace MyCoroutine


/*
// 恢复从协程batch中协程的调用，只能在主协程中调用
int CoroutineResumeInBatch(Schedule& schedule, int id);
// 恢复被插入batch卡点的从协程的调用，只能在主协程中调用
int CoroutineResumeBatchFinish(Schedule& schedule);
// 判断当前从协程是否在batch中
bool CoroutineIsInBatch(Schedule& schedule);
// 协程栈使用检测
int CoroutineStackCheck(Schedule& schedule, int id);

// 初始化一个批量执行的上下文
int BatchInit(Schedule& schedule);
// 在批量执行上下文中添加要执行的任务
void BatchAdd(Schedule& schedule, int batchId, Entry entry, void* arg, uint32_t priority = 0);
// 执行批量操作
void BatchRun(Schedule& schedule, int batchId);
*/