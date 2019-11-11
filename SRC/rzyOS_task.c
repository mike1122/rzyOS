#include "rzyOS.h"

void task_init(task_tcb_s *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x03;
	*(--stack) = (unsigned long)0x02;
	*(--stack) = (unsigned long)0x01;
	*(--stack) = (unsigned long)param;
	
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x09;
	*(--stack) = (unsigned long)0x08;
	*(--stack) = (unsigned long)0x07;
	*(--stack) = (unsigned long)0x06;
	*(--stack) = (unsigned long)0x05;
	*(--stack) = (unsigned long)0x04;
	
	task -> stack = stack;
	node_init(&(task -> link_node));
	task -> delayTicks = 0;
	task -> prio = prio;
	task -> task_status = RZYOS_TASK_STATUS_READY;
	task -> slice = RZYOS_SLICE_MAX;
	node_init(&(task -> delay_node));
	task -> suspend_count = 0;
	task -> clean = (void (*) (void *))0;
	task -> clean_param = (void *)0;
	task -> request_delete_flag = 0;
	
	task_insert_ready_list(task);
}

void rzyOS_task_suspend(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	//�����������ʱ����, ˵�����ھ�������, �򲻽��й������
	if (!(task -> task_status & RZYOS_TASK_STATUS_DELAY))
	{
		//ÿ�α����ù������,��Ҫ��suspend_count��1
		task -> suspend_count ++;
		//���suspend_count��1, ˵�������ڵ�һ�ι���, ��Ҫ�����л����� , ���ڵ���2ze������Ҫ�л�����
		if (1 == task -> suspend_count)
		{
			//���ù����־
			task -> task_status |= RZYOS_TASK_STATUS_SUSPEND;
			//�Ӿ����б����Ƴ�
			task_remove_ready_list(task);
			//�����ǰ�������Լ��Ļ�, ����Ҫ���������л�
			if (task == currentTask)
			{
				task_schedule();
			}
		}
	}
	
	task_exit_critical(status);
}

void rzyOS_task_wakeup(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	if (task -> task_status & RZYOS_TASK_STATUS_SUSPEND)
	{
		//ÿ�ε��ý�Һ���,��Ҫ��1
		task -> suspend_count --;
		//��suspend_countΪ0ʱ˵����Ҫ��������б���
		if (0 == task -> suspend_count)
		{
			task -> task_status &= ~RZYOS_TASK_STATUS_SUSPEND;
			task_insert_ready_list(task);
			//�����ѵ��������ȼ��п������, �������Ȳþ��Ƿ���Ҫ�л�
			task_schedule();
		}
	}
	
	task_exit_critical(status);
}

void rzyOS_task_clean_callback(task_tcb_s *task, void (*clean)(void *param), void *param)
{
	task -> clean = clean;
	task -> clean_param = param;
}

void rzyOS_task_force_delete(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	if (task -> task_status & RZYOS_TASK_STATUS_DELAY)
	{
		rzyOS_task_delay_list_remove(task);
	}
	else if (!(task -> task_status & RZYOS_TASK_STATUS_SUSPEND))
	{
		rzyOS_task_ready_list_remove(task);
	}
	
	if (task -> clean)
	{
		task -> clean(task -> clean_param);
	}
	
	if (currentTask == task)
	{
		task_schedule();
	}
	
	task_exit_critical(status);
}

void rzyOS_task_request_delete(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	task -> request_delete_flag = 1;
	
	task_exit_critical(status);
}

uint8_t rzyOS_task_request_delete_check(void)
{
	uint8_t delete_status;
	
	uint32_t status = task_enter_critical();
	
	delete_status = currentTask -> request_delete_flag;
	
	task_exit_critical(status);
	
	return delete_status;
}

void rzyOS_task_delete_self(void)
{
	uint32_t status = task_enter_critical();
	
	rzyOS_task_ready_list_remove(currentTask);
	
	if (currentTask -> clean)
	{
		currentTask -> clean(currentTask -> clean_param);
	}
	
	task_schedule();
	
	task_exit_critical(status);	
}
