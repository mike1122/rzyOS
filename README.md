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

-	**双向链表** -> *在TCB中加入延时节点, 在systick的中断里，会把多任务的延时字段， 使用双向链表做递减*
-	**双向链表** -> *在TCB中加入任务连接节点(就绪节点), 每个优先级中都有一个就绪链表， 在task schedule函数中， 找最高优先级做调度*

# 2019-12-06
*toworrow i will go to Singapore for work and life, peace*

# 2019-12-10
*now in Singapore, new stage begin, rzyOS will continue*

# 2019-12-13
## rzyOS 数据结构--机制

-	*事件的本质仍然是通过双向链表去独立出一套机制， 把需要使用事件的任务管理起来. 组织任务的事件可以有很多种， 不仅仅是信号.*
-	**双向链表** --> *引入事件等待， 事件触发唤醒机制*

# 2020-1-3
## 事件控制块是

-	*事件控制块是 ： 信号量， 消息队列， 事件标志 的基础组件*

# 2020-1-3
## 信号量

-	*信号量就是带计数器的事件控制块*


# 2020-1-3
## 状态查询

-	*关于状态查询的套路 : 就是定义一个info的结构， 然后进入临界区， 把感兴趣的字段复制进去就ok了*