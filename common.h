#pragma once

#include <ucontext.h>

#include <algorithm>
#include <functional>
#include <unordered_map>

using namespace std;

namespace MyCoroutine {
constexpr int32_t kInvalidCid = -1;           // 无效的从协程id
constexpr int32_t kInvalidBid = -1;           // 无效的批量执行id
constexpr int32_t kStackSize = 64 * 1024;     // 协程栈默认大小为 64K
constexpr int32_t kMaxBatchSize = 5120;       // 允许创建的最大批量执行池大小
constexpr int32_t kMaxCoroutineSize = 10240;  // 允许创建的最大协程池大小
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
typedef struct LocalVariable {
  void *data{nullptr};
  function<void(void *)> free{nullptr};  // 用于释放本地协程变量的内存
} LocalVariable;

// 批量执行结构体
typedef struct Batch {
  int32_t bid{kInvalidBid};                         // 批量执行id
  State state{State::kIdle};                        // 批量执行的状态
  int32_t parent_cid{kInvalidCid};                  // 父的从协程id
  unordered_map<int32_t, bool> child_cid_2_finish;  // 标记子的从协程是否执行完
} Batch;

// 协程结构体
typedef struct Coroutine {
  int32_t cid{kInvalidCid};                    // 从协程id
  State state{State::kIdle};                   // 从协程当前的状态
  function<void()> entry{nullptr};             // 从协程入口函数
  ucontext_t ctx;                              // 从协程执行上下文
  uint8_t *stack{nullptr};                     // 每个协程独占的协程栈，动态分配
  unordered_map<void *, LocalVariable> local;  // 协程本地变量，key是协程变量的内存地址
  int32_t relate_bid{kInvalidBid};             // 关联的批量执行id
} Coroutine;
};  // namespace MyCoroutine
