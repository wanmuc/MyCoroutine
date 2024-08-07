#pragma once

#include <functional>
#include <ucontext.h>

namespace MyCoroutine {

constexpr int32_t kInvalidCid = -1; // 无效的从协程id
/**
 * 从协程的状态机转移如下所示：
 *  kIdle->kReady
 *  kReady->kRun
 *  kRun->kSuspend
 *  kSuspend->kRun
 *  kRun->kIdle
 */
enum class State {
  kIdle = 1,    // 空闲
  kReady = 2,   // 就绪
  kRun = 3,     // 运行
  kSuspend = 4, // 挂起
};

typedef struct Coroutine {
  int32_t cid{kInvalidCid};             // 从协程id
  State state{State::kIdle};            // 从协程当前的状态
  std::function<void()> entry{nullptr}; // 从协程入口函数
  ucontext_t ctx;                       // 从协程执行上下文
  uint8_t *stack{nullptr}; // 每个协程独占的协程栈，动态分配
} Coroutine;

}; // namespace MyCoroutine
