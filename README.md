﻿# rzyOS
**rzyOS a simple OS , create by renzhongyu**
*筹备了很久的操作系统, 开始了*

# 2019-09-21 
*硬件调试不是很顺利,准备先完成rzyOS的基本功能,再结合硬件*

# 2019-11-06 
## rzyOS 数据结构--机制

-	**中断屏蔽寄存器状态保存与恢复机制** --> *临界区保护 (退出中断)机制*

-	**使用全局计数值做为** --> *使能失能任务的 调度锁*

-	**位图查询 + 任务函数指针数组** --> *任务优先级调度 策略*

-	**双向链表** -> *在TCB中加入延时节点,多任务延时使用双向链表做递减*
-	**双向链表** -> *在TCB中加入延时节点,多任务延时使用双向链表做递减*

# 2019-12-06
*toworrow i will go to Singapore for work and life, peace*

# 2019-12-10
*now in Singapore, new stage begin, rzyOS will continue*

# 2019-12-13
## rzyOS 数据结构--机制

-	**双向链表** --> *引入事件等待，事件触发唤醒机制*
