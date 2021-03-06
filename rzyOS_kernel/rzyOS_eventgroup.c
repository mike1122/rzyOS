#include "rzyOS_eventgroup.h"

#if RZYOS_ENABLE_EVENTGROUP == 1

//时间组初始化函数
//parameter : 
//rzyOS_flag_group_s *rzyOS_flag_group : 事件标志组
//uint32_t flags : 标志组类型
void rzyOS_flag_group_init(rzyOS_flag_group_s *rzyOS_flag_group, uint32_t flags)
{
	rzyOS_event_init(&(rzyOS_flag_group -> rzyOS_ecb), event_type_flag_group);
	rzyOS_flag_group -> flag = flags;
}

//事件检查函数
//parameter : 
//uint32_t type : 等待的事件类型(标志组合)(uint8_t)
//uint32_t *flag : 任务期望等待的事件类型标志
static uint32_t rzyOS_flag_group_check(rzyOS_flag_group_s *rzyOS_flag_group, uint32_t type, uint32_t *flag)
{
	//任务希望等待的标志
	uint32_t src_flag = *flag;
	//判断是设置还是清零
	uint32_t is_set = type & FLAGGROUP_SET;
	//判断是不是等待所有的标志
	uint32_t is_all = type & FLAGGROUP_ALL;
	//判断满足条件后是否需要把标志消耗掉
	uint32_t is_consume = type & FLAGGROUP_CONSUME;

	//对当前发生的标志位(设置或清零)进行计算
	//is_set ? : 对置位进行统计
	//(rzyOS_flag_group -> flag & src_flag) : 当前事件组与任务期望等待的标志类型所匹配的置位的位
	//(~rzyOS_flag_group -> flag & src_flag) : 当前事件组与任务期望等待的标志类型所匹配的为零的位
	uint32_t cal_flag = is_set ? (rzyOS_flag_group -> flag & src_flag) : ((~rzyOS_flag_group -> flag) & src_flag);

	//判断相应的标志是否已经满足
	//所有标志 && 计算的与请求的一样 || 任意标志 && 计算标志有统计到的
	if (((is_all != 0) && (cal_flag == src_flag)) || ((is_all == 0) && (cal_flag != 0)))
	{
		//是否需要消耗掉标志
		if (is_consume)
		{
			//如果是置位 ,则清零
			if (is_set)
			{
				rzyOS_flag_group -> flag &= ~src_flag;
			}
			//如果是清零, 则置位
			else
			{
				rzyOS_flag_group -> flag |= src_flag;
			}
		}

		//传递计算的标志位
		*flag = cal_flag;

		return error_no_error;
	}

	//不满足
	//无资源
	*flag = cal_flag;

	return error_resource_unvaliable;
}


//事件组等待函数(阻塞)
//parameter : 
//uint32_t wait_type : 等待的事件类型(uint8_t)
//uint32_t request_flag : 请求的标志
//uint32_t result_flag : 等待标志的结果
//return : emnu rzyOS_error_e
//----------------------------------------------------
//example : 
//rzyOS_flag_group_wait(flag_group_1, FLAGGROUP_CLEAR_ALL, 0x06, &result_flag, wait_ticks);
//等待flag_group_1中第1位和第2位的标志全部清零,并把结果放入result_flag,超时等待wait_ticks
//
//rzyOS_flag_group_wait(flag_group_1, FLAGGROUP_CLEAR_ALL | FLAGGROUP_CONSUME, 0x06, &result_flag, wait_ticks);
//等待flag_group_1中第1位和第2位的标志全部清零, 条件满足后消耗(此处为置位),并把结果放入result_flag,超时等待wait_ticks
//----------------------------------------------------
uint32_t rzyOS_flag_group_wait(rzyOS_flag_group_s *rzyOS_flag_group, uint32_t wait_type, uint32_t request_flag, uint32_t *result_flag, uint32_t wait_ticks)
{
	//rzyOS_flag_group_check()函数返回值
	uint32_t result;
	//请求的标志
	uint32_t flags = request_flag;

	uint32_t status = task_enter_critical();

	//检查事件标志
	result = rzyOS_flag_group_check(rzyOS_flag_group, wait_type, &flags);

	//返回结果有问题的情况, 需要插入到事件等待队列
	if (result != error_no_error)
	{
		//设置当前任务等待的类型
		currentTask -> wait_flag_type = wait_type;
		//等待的事件标志
		currentTask -> event_flag = request_flag;

		//插入到时间控制块中
		rzyOS_event_wait(&(rzyOS_flag_group -> rzyOS_ecb), currentTask, (void *)0, event_type_flag_group, wait_ticks);

		task_exit_critical(status);

		//进行调度
		task_schedule();

		//取出任务的等待标志
		*result_flag = currentTask -> event_flag;
		//等待事件的结果
		result = currentTask -> wait_event_result;
	}
	//返回的结果满足要求
	else
	{
		//取出等待标志结果
		*result_flag = flags;
		task_exit_critical(status);
	}

	return result;
}

//事件组等待函数(非阻塞)
//parameter : 
//uint32_t wait_type : 等待的事件组类型
//uint32_t request_flag : 请求的标志
//uint32_t result_flag : 等待标志的结果
uint32_t rzyOS_flag_group_no_wait(rzyOS_flag_group_s *rzyOS_flag_group, uint32_t wait_type, uint32_t request_flag, uint32_t *result_flag)
{
	uint32_t flags = request_flag;

	uint32_t status = task_enter_critical();

	//检查事件标志
	rzyOS_flag_group_check(rzyOS_flag_group, wait_type, &flags);

	task_exit_critical(status);

	//取出等待标志结果
	*result_flag = flags;

	return error_no_error;
}

//时间组通知函数
//parameter : 
//uint8_t is_set : 标志类型(event_flag)
//uint32_t flag : 事件标志组标志值
//----------------------------------------------------
//example : 
//rzyOS_flag_group_post(flag_group_1, 0, 0x07);
//对 flag_group_1 事件标志组的第0位第1位第2位做清零操作
//----------------------------------------------------
void rzyOS_flag_group_post(rzyOS_flag_group_s *rzyOS_flag_group, uint8_t is_set, uint32_t flag)
{
	list_t *wait_list;
	node_t *node;
	node_t *next_node;
	uint8_t need_sched;

	uint32_t status = task_enter_critical();

	//对事件组标志值清零还是置位
	if (is_set)
	{
		rzyOS_flag_group -> flag |= flag;
	}
	else
	{
		rzyOS_flag_group -> flag &= ~flag;
	}

	//取出等待队列的地址
	wait_list = &(rzyOS_flag_group -> rzyOS_ecb.wait_list);

	//遍历等待列表
	for (node = wait_list -> head_node.next_node; node != &(wait_list -> head_node); node = next_node)
	{
		uint32_t result;
		//获取等待列表中等待节点的任务指针
		task_tcb_s *task_tcb = node_parent(node, task_tcb_s, link_node);
		//取出任务希望等待的任务标志
		uint32_t flags = task_tcb -> event_flag;
		next_node = node -> next_node;

		//检查任务等待的标志类型与事件标志是否已经满足
		result = rzyOS_flag_group_check(rzyOS_flag_group, task_tcb -> wait_flag_type, &flags);

		//没有错误,满足条件
		if (error_no_error == result)
		{
			//则把任务等待的事件标志赋回给event_flag
			task_tcb -> event_flag = flags;
			//唤醒当前任务,并插入时间组等待队列
			rzyOS_event_wakeup_appoint_task(&(rzyOS_flag_group -> rzyOS_ecb), task_tcb, (void *)0, error_no_error);
			//需要检查优先级,进行调度
			need_sched = 1;
		}
	}

	if (need_sched)
	{
		task_schedule();
	}

	task_exit_critical(status);
}

//事件标志组删除函数
//return : 移除的任务个数
uint32_t rzyOS_flag_group_destroy(rzyOS_flag_group_s *rzyOS_flag_group)
{
	uint32_t status = task_enter_critical();

	uint32_t count = rzyOS_event_remove_all(&(rzyOS_flag_group -> rzyOS_ecb), (void *)0, error_delete);

	task_exit_critical(status);

	if (count > 0)
	{
		task_schedule();
	}

	return count;
}

//事件组消息获取函数
void rzyOS_flag_group_get_info(rzyOS_flag_group_s *rzyOS_flag_group, rzyOS_flag_group_info_s *rzyOS_flag_group_info)
{
	uint32_t status = task_enter_critical();

	rzyOS_flag_group_info -> flag = rzyOS_flag_group -> flag;
	rzyOS_flag_group_info -> task_count = rzyOS_event_wait_count(&(rzyOS_flag_group -> rzyOS_ecb));

	task_exit_critical(status);
}

#endif
