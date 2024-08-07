#include <assert.h>

#include "schedule.h"

namespace MyCoroutine {
void Schedule::CoroutineLocalSet(void* key, const LocalVariable& local_variable) {
  assert(not is_master_);
  auto iter = coroutines_[slave_cid_]->local.find(key);
  if (iter != coroutines_[slave_cid_]->local.end()) {
    iter->second.free(iter->second.data);  // 之前有值，则要先释放空间
  }
  coroutines_[slave_cid_]->local[key] = local_variable;
}

bool Schedule::CoroutineLocalGet(void* key, LocalVariable& local_variable) {
  assert(not is_master_);
  auto iter = coroutines_[slave_cid_]->local.find(key);
  assert(iter != coroutines_[slave_cid_]->local.end());
  local_variable = iter->second;
  return true;
}
}  // namespace MyCoroutine