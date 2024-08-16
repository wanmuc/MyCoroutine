#pragma once

#include "common.h"

namespace MyCoroutine {
/**
 * SingleFlight的状态，SingleFlight的状态转移如下：
 * kInit -> kCalling
 * kCalling -> kFinish
 * kFinish -> kCalling
 */
enum class SingleFlightState {
  kInit = 1,     // 初始化
  kCalling = 2,  // 调用中
  kFinish = 3,   // 完成
};

class SingleFlight {
public:
private:
  SingleFlightState state_{SingleFlightState::kInit};
};
}  // namespace MyCoroutine
