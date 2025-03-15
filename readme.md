# MyCoroutine

> MyCoroutine是使用C++11编写的Linux平台下的协程库。

注意：因为本协程库的实现依赖Linux平台下的三个库函数，getcontext，makecontext和swapcontext，所以本协程库只支持Linux平台，不支持跨平台。

# 目录结构说明

本仓库的目录结构说明如下。

- demo目录：所有示例程序的目录，每个子目录对应一个示例程序
- sync目录：同步原语代码目录
- test目录：单元测试代码目录，里面有每个特性的测试代码
- batch.cpp：批量执行的实现
- common.h：公共结构体、枚举、常量的定义
- coroutine.cpp：协程核心API的实现
- localvariable.h、localvariable.cpp：协程本地变量的实现
- mycoroutine.h：协程库主要接口的声明

# 协程调度模型

本协程库使用的是1对1的调度模型，也就是说一个协程只会在一个线程上被调度。

因为同一个线程上的多个协程之间是串行执行的，它们之间不存在并发读写数据的问题。

虽然没有了并发读写数据的问题，但是需要处理多个请求之间读写共享数据的时序问题。

多个请求之间读写共享数据的时序问题也比较好解决，通过状态机来处理即可。

注意：如果在一个进程中创建多个线程，并在每个线程中使用了独立的协程池，则依然需要处理多线程的并发读写数据的问题。

# 实现的特性

- 协程创建、协程恢复、协程挂起、协程调度

- 协程本地变量

- 协程批量执行

- 协程互斥锁

- 协程读写锁

- 协程条件变量

- 协程信号量

- 协程sleep

- Channel

- SingleFlight

- CallOnce

# 快速开始

下面展示了如何使用本协程库来实现hello world的打印。

```C++
#include "mycoroutine.h"
#include <iostream>

using namespace std;
using namespace MyCoroutine;

void HelloWorld(Schedule &schedule) {
  cout << "hello ";
  schedule.CoroutineYield();
  cout << "world" << endl;
}

int main() {
  // 创建一个协程调度对象，并自动生成大小为1024的协程池
  Schedule schedule(1024);
  // 创建一个从协程，并手动调度
  {
    int32_t cid = schedule.CoroutineCreate(HelloWorld, ref(schedule));
    schedule.CoroutineResume(cid);
    schedule.CoroutineResume(cid);
  }
  // 创建一个从协程，并自行调度
  {
    schedule.CoroutineCreate(HelloWorld, ref(schedule));
    schedule.Run();  // Run函数完成从协程的自行调度，直到所有的从协程都执行完
  }
  return 0;
}
```

本仓库的所有代码都是使用make命令来编译，采用的是通用的makefile编译脚本。

更多的示例代码，详见demo子目录内容，demo目录下的每个子目录都是一个单独的示例程序。

# 知乎上介绍本库的系列文章

- [协程必知必会-系列1-协程是什么](https://zhuanlan.zhihu.com/p/717419078)
- [协程必知必会-系列2-协程如何实现](https://zhuanlan.zhihu.com/p/719298566)
- [协程必知必会-系列3-批量并发执行](https://zhuanlan.zhihu.com/p/721026660)
- [协程必知必会-系列4-协程本地变量](https://zhuanlan.zhihu.com/p/721890237)
- [协程必知必会-系列5-协程互斥锁](https://zhuanlan.zhihu.com/p/722216866)
- [协程必知必会-系列6-协程读写锁](https://zhuanlan.zhihu.com/p/911995023)
- [协程必知必会-系列7-协程条件变量](https://zhuanlan.zhihu.com/p/998411846)
- [协程必知必会-系列8-协程信号量](https://zhuanlan.zhihu.com/p/1210540508)
- [协程必知必会-系列9-Channel](https://zhuanlan.zhihu.com/p/1355321479)
- [协程必知必会-系列10-SingleFlight](https://zhuanlan.zhihu.com/p/1620898868)
- [协程必知必会-系列11-CallOnce](https://zhuanlan.zhihu.com/p/1673369654)
- [协程必知必会-加餐1-协程Sleep](https://zhuanlan.zhihu.com/p/6177198171)


# 微信公众号
欢迎关注微信公众号「Linux后端开发工程实践」，第一时间获取最新文章！扫码即可订阅。
![img.png](https://github.com/wanmuc/MyCoroutine/blob/main/mp_account.png#pic_center=660*180)