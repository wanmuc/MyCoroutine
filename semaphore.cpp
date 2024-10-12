#include "semaphore.h"

#include "mycoroutine.h"

namespace MyCoroutine {
void Schedule::CoSemaphoreInit(CoSemaphore &semaphore, int64_t value) {
  semaphore.value = value;
  semaphores_.insert(&semaphore);
}

void Schedule::CoSemaphoreClear(CoSemaphore &semaphore) { semaphores_.erase(&semaphore); }

void Schedule::CoSemaphorePost(CoSemaphore &semaphore) {
  // TODO
}

void Schedule::CoSemaphoreWait(CoSemaphore &semaphore) {
  // TODO
}

int Schedule::CoSemaphoreResume() {
  assert(is_master_);
  int count = 0;
  // TODO
  return count;
}

void Semaphore::Post() { schedule_.CoSemaphorePost(co_semaphore_); }

void Semaphore::Wait() { schedule_.CoSemaphoreWait(co_semaphore_); }
}  // namespace MyCoroutine