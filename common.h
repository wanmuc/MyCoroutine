#pragma once

#include <ucontext.h>

#include <functional>
#include <unordered_map>

using namespace std;

namespace MyCoroutine {
constexpr int32_t kInvalidCid = -1;  // 无效的从协程id
/**
 * 从协程的状态机转移如下所示：
 *  kIdle->kReady
 *  kReady->kRun
 *  kRun->kSuspend
 *  kSuspend->kRun
 *  kRun->kIdle
 */
enum class State {
  kIdle = 1,     // 空闲
  kReady = 2,    // 就绪
  kRun = 3,      // 运行
  kSuspend = 4,  // 挂起
};

// 协程本地变量结构体
typedef struct LocalData {
  void *data{nullptr};
  function<void(void *)> free{nullptr};  // 用于释放本地协程变量的内存
} LocalData;

// 协程结构体
typedef struct Coroutine {
  int32_t cid{kInvalidCid};                // 从协程id
  State state{State::kIdle};               // 从协程当前的状态
  function<void()> entry{nullptr};         // 从协程入口函数
  ucontext_t ctx;                          // 从协程执行上下文
  uint8_t *stack{nullptr};                 // 每个协程独占的协程栈，动态分配
  unordered_map<void *, LocalData> local;  // 协程本地变量，key是协程变量的内存地址
} Coroutine;
};  // namespace MyCoroutine
