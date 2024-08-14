#pragma once
#include <assert.h>

#include <list>

#include "common.h"

namespace MyCoroutine {
// 协程调度器
class Schedule {
public:
  explicit Schedule(int32_t coroutine_count, int32_t max_task_in_batch = 0);
  ~Schedule();
  
  // 从协程的创建函数，通过模版函数，可以支持不同原型的函数，作为从协程的执行函数
  template <typename Function, typename... Args>
  int32_t CoroutineCreate(Function &&func, Args &&...args) {
    int32_t cid = 0;
    for (cid = 0; cid < coroutine_count_; cid++) {
      if (coroutines_[cid]->state == State::kIdle) {
        break;
      }
    }
    if (cid >= coroutine_count_) {
      return kInvalidCid;
    }
    Coroutine *routine = coroutines_[cid];
    function<void()> entry =
        bind(forward<Function>(func), forward<Args>(args)...);
    CoroutineInit(routine, entry);
    return cid;
  }

  void Run();  // 协程调度执行
  void CoroutineYield();  // 从协程让出cpu执行权
  void CoroutineResume(int32_t cid);  // 主协程唤醒指定的从协程

  
  void CoroutineResume4BatchStart(int32_t cid);  // 主协程唤醒指定从协程中的批量执行中的子从协程
  void CoroutineResume4BatchFinish();  // 主协程唤醒被插入批量执行的父从协程的调用

  void LocalVariableSet(void *key, const LocalVariable &local_variable);  // 设置协程本地变量
  bool LocalVariableGet(void *key, LocalVariable &local_variable);  // 获取协程本地变量
  
  int32_t BatchCreate();  // 创建一个批量执行
  // 在批量执行中添加任务
  template <typename Function, typename... Args>
  void BatchAdd(int32_t bid, Function &&func, Args &&...args) {  
    assert(not is_master_);                       // 从协程中才可以调用
    assert(bid >= 0 && bid < kMaxBatchSize);      // 校验bid的合法性
    assert(batchs_[bid]->state == State::kReady); // batch必须是ready的状态
    assert(batchs_[bid]->parent_cid == slave_cid_); // 父的从协程id必须正确
    int32_t cid =
        CoroutineCreate(forward<Function>(func), forward<Args>(args)...);
    assert(cid != kInvalidCid);
    coroutines_[cid]->relate_bid = bid;            // 设置关联的bid
    batchs_[bid]->child_cid_2_finish[cid] = false; // 子的从协程都没执行完
  }

  void BatchRun(int32_t bid);    // 运行批量执行
  bool IsBatchDone(int32_t bid); // 批量执行是否完成

  void CoMutexInit(CoMutex &co_mutex);   // 互斥锁初始化
  void CoMutexClear(CoMutex &co_mutex);  // 互斥锁清理
  void CoMutexLock(CoMutex &co_mutex);   // 锁定互斥锁
  void CoMutexUnLock(CoMutex &co_mutex); // 解锁互斥锁

private:
  static void CoroutineRun(Schedule *schedule, Coroutine *routine);  // 从协程的执行入口
  void CoroutineInit(Coroutine *routine, std::function<void()> entry); // 从协程的初始化

private:
  ucontext_t main_;      // 保存主协程的上下文
  bool is_master_{true}; // 是否主协程
  int32_t slave_cid_{kInvalidCid}; // 运行中的从协程的id（运行从协程时才有效）
  int32_t not_idle_count_{0};      // 运行中和挂起的从协程数
  int32_t coroutine_count_{0};     // 从协程总数
  int32_t stack_size_{kStackSize}; // 从协程栈大小，单位字节
  Coroutine *coroutines_[kMaxCoroutineSize]; // 从协程数组池
  Batch *batchs_[kMaxBatchSize];             // 批量执行数组池
  list<int> batch_finish_cid_list_; // 完成了批量执行的关联的协程的id
  CoMutexManage mutex_manage_;      // 互斥锁管理
};
} // namespace MyCoroutine