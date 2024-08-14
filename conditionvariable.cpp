#include "conditionvariable.h"
#include "mycoroutine.h"

namespace MyCoroutine {
void Schedule::CoCondInit(CoCond &co_cond) {
  co_cond.state = CondState::kNotifyNone;
  conds_.insert(&co_cond);
}

void Schedule::CoCondClear(CoCond &co_cond) {
  conds_.erase(&co_cond);
}

void Schedule::CoCondWait(CoCond &co_cond, std::function<bool()> pred) {
  while (not pred()) {
    assert(not is_master_);
    // 更新因为的等待条件变量而被挂起的从协程
    co_cond.suspend_cid_set.insert(slave_cid_);
    CoroutineYield();
  }
}

void Schedule::CoCondNotifyOne(CoCond &co_cond) {
  if (co_cond.state == CondState::kNotifyNone) {
    co_cond.state = CondState::kNotifyOne;
  }
}

void Schedule::CoCondNotifyAll(CoCond &co_cond) {
  co_cond.state = CondState::kNotifyAll;
}

void Schedule::CoCondResume() {
  for (auto *cond : conds_) {
    if (cond->state == CondState::kNotifyNone) continue; // 没有通知，不需要互相等待的从协程
    // 通知了，但是没有挂起的从协程，也不需要唤醒，注意这里不调整通知的状态
    if (cond->suspend_cid_set.size() <= 0) continue;
    // 有挂起的协程才调整通知的状态
    if (cond->state == CondState::kNotifyOne) {
      int32_t cid = *cond->suspend_cid_set.begin();
      cond->suspend_cid_set.erase(cid);
      CoroutineResume(cid);  // 每次只能唤醒等待队列中的一个从协程，采用先进先出的策略
    } else if (cond->state == CondState::kNotifyAll) {
      // 唤醒所有等待的从协程
      unordered_set<int32_t> cid_set = cond->suspend_cid_set;
      cond->suspend_cid_set.clear();  // 需要在这里就清空这个集合，因为被唤醒的从协程可能重新再阻塞
      for (const auto &cid : cid_set) {
        CoroutineResume(cid);
      }
    } else {
      assert(0);
    }
    cond->state = CondState::kNotifyNone;
  }
}
} // namespace MyCoroutine