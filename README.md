﻿# rzyOS 被 RenZhongYu[任仲雨] 所创造
*rzyOS 是基于宏内核设计的操作系统*
*rzyOS Version : V1.0*

rzyOS 采用抢占式优先级+时间片轮转的调度策略，支持硬件浮点，task堆栈统计，CPU使用率统计。
最多支持32个task，支持信号量，消息队列，互斥锁，内存管理，工作队列，事件组。支持优先级继承

rzyOS 支持linux与windows开发环境，拥有makefile 与 cmake 构建方式，以及MDK编译方式


## 目前支持的架构
```
Cortex-M3
Cortex-M4
```


## 目前支持的测试平台
```
stm32f4discovery
MDK simulation
```

## 支持的开发环境
```
Linux(推荐)
Windows
```

### Linux
```
cmake：
1.mkdir build
2.cd build & cmake ..
3.make
```

```
makefile:
1.cd src/board/stm32f4discovery
2.make
3.make flash
```

### Windows
```
1.进入keil_proj目录
2.选择平台(推荐proj_stm32f407)
```
