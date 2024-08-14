#pragma once

#include "common.h"
#include "mycoroutine.h"

namespace MyCoroutine {
// 协程本地变量模版类封装
template <typename Type> class CoroutineLocalVariable {
public:
  CoroutineLocalVariable(Schedule &schedule) : schedule_(schedule) {}
  static void free(void *data) {
    if (data)
      delete (Type *)data;
  }
  void Set(Type value) {
    Type *data = new Type(value);
    MyCoroutine::LocalVariable local_variable;
    local_variable.data = data;
    local_variable.free = free;
    schedule_.LocalVariableSet(this, local_variable);
  }
  Type &Get() {
    MyCoroutine::LocalVariable local_variable;
    bool result = schedule_.LocalVariableGet(this, local_variable);
    assert(result == true);
    return *(Type *)local_variable.data;
  }

private:
  Schedule &schedule_;
};
} // namespace MyCoroutine