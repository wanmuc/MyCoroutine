#include "callonce.h"

namespace MyCoroutine {
void Schedule::CoCallOnceInit(CoCallOnce& callonce) {
  callonce.state = CallOnceState::kInit;   // CallOnce状态
  unordered_set<int32_t> suspend_cid_set;  // 被挂起的从协程id查重集合
  assert(callonces_.find(&callonce) == callonces_.end());
  callonces_.insert(&callonce);
}

void Schedule::CoCallOnceClear(CoCallOnce& callonce) { callonces_.erase(&callonce); }

int Schedule::CoCallOnceResume() {
  assert(is_master_);
  int count = 0;
  for (auto* callonce : callonces_) {
    if (callonce->state != CallOnceState::kFinish) continue;  // 没执行完，不需要唤醒其他从协程
    if (callonce->suspend_cid_list.size() <= 0) continue;     // 是没有挂起的从协程，也不需要唤醒
    // 唤醒所有等待的从协程
    for (const auto& cid : callonce->suspend_cid_set) {
      CoroutineResume(cid);
      count++;
    }
    callonce->suspend_cid_set.clear();
  }
  return count;
}
}  // namespace MyCoroutine
