#pragma once

#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
// 协程信号量类封装
class Semaphore {
 public:
   Semaphore(Schedule &schedule, int64_t value) : schedule_(schedule) {
     schedule_.CoSemaphoreInit(co_semaphore_, value);
   }
   ~Semaphore() { schedule_.CoSemaphoreClear(co_semaphore_); }

   void Post();
   void Wait();

 private:
  CoCond co_semaphore_;
  Schedule &schedule_;
};
}  // namespace MyCoroutine