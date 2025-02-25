#include "sync/callonce.h"

namespace MyCoroutine {
void Schedule::CoCallOnceInit(CoCallOnce& callonce) {
  callonce.state = CallOnceState::kInit;  // CallOnce状态
  assert(cosync_.call_onces.find(&callonce) == cosync_.call_onces.end());
  cosync_.call_onces.insert(&callonce);
}

void Schedule::CoCallOnceClear(CoCallOnce& callonce) { cosync_.call_onces.erase(&callonce); }

int Schedule::CoCallOnceResume() {
  assert(is_master_);
  int count = 0;
  for (auto* call_once : cosync_.call_onces) {
    if (call_once->state != CallOnceState::kFinish) continue;  // 没执行完，不需要唤醒其他从协程
    if (call_once->suspend_cid_set.size() <= 0) continue;      // 是没有挂起的从协程，也不需要唤醒
    auto cid_set = call_once->suspend_cid_set;
    // 唤醒所有等待的从协程
    for (const auto cid : cid_set) {
      CoroutineResume(cid);
      count++;
    }
    assert(call_once->suspend_cid_set.size() == 0);
  }
  return count;
}
}  // namespace MyCoroutine
