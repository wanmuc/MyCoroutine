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

class Coroutine {
public:
  void Entry() {
    entry_();
    state_ = State::kIdle;
  }

private:
  State state_{State::kIdle};            // 从协程当前的状态
  std::function<void()> entry_{nullptr}; // 从协程入口函数
  ucontext_t ctx_;                       // 从协程执行上下文
  uint8_t *stack_{nullptr}; // 每个协程独占的协程栈，动态分配
};

}; // namespace MyCoroutine
