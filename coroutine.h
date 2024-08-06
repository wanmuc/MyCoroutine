#pragma once

#include <functional>
#include <ucontext.h>

namespace MyCoroutine {
/**
 * 协程的状态，协程的状态转移如下：
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
  State state{State::kIdle};            // 从协程当前的状态
  std::function<void()> entry{nullptr}; // 从协程入口函数
  ucontext_t ctx;                       // 从协程执行上下文
  uint8_t *stack{nullptr}; // 每个协程独占的协程栈，动态分配
} Coroutine;

}; // namespace MyCoroutine
