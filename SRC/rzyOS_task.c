#include "rzyOS.h"

//task��ʼ��
void task_init(task_tcb_s *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack_bottom, uint32_t task_size)
{
	//����ջ��(�ߵ�ַ)
	uint32_t *stack_top;
	//ջ�׸�ֵ(�͵�ַ)
	task -> task_bottom_base = stack_bottom;
	//ջ��С
	task -> task_stack_size = task_size;
	//ջ����
	memset(stack_bottom, 0, task_size);

	//����ջ����ַ
	stack_top = stack_bottom + task_size / sizeof(tTaskStack);

	//��ʼ�������ջ����Ӧ��ͨ�üĴ���
	*(--stack_top) = (unsigned long)(1 << 24);
	*(--stack_top) = (unsigned long)entry;
	*(--stack_top) = (unsigned long)0x14;
	*(--stack_top) = (unsigned long)0x12;
	*(--stack_top) = (unsigned long)0x03;
	*(--stack_top) = (unsigned long)0x02;
	*(--stack_top) = (unsigned long)0x01;
	*(--stack_top) = (unsigned long)param;
	
	*(--stack_top) = (unsigned long)0x11;
	*(--stack_top) = (unsigned long)0x10;
	*(--stack_top) = (unsigned long)0x09;
	*(--stack_top) = (unsigned long)0x08;
	*(--stack_top) = (unsigned long)0x07;
	*(--stack_top) = (unsigned long)0x06;
	*(--stack_top) = (unsigned long)0x05;
	*(--stack_top) = (unsigned long)0x04;
	
	//�����ջָ�봫��
	task -> stack = stack_top;
	//��ʼ������tcb
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
	
	//��ʼ�����������������ʱ���еȴ�����
	task_insert_ready_list(task);
}

//����Ĺ���
void rzyOS_task_suspend(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	//�����������ʱ����, ˵�����ھ�������, �򲻽��й��������Ŀǰ���Դ�����ʱ���е�������й���
	if (!(task -> task_status & RZYOS_TASK_STATUS_DELAY))
	{
		//ÿ�α����ù������,��Ҫ��suspend_count��1
		task -> suspend_count ++;
		//���suspend_count��1, ˵�������ڵ�һ�ι���, ��Ҫ�����л����� , ���ڵ���2������Ҫ�л�����
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

//����Ļ���
void rzyOS_task_wakeup(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	//��������ڹ���״̬
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

//��������ص�����
void rzyOS_task_clean_callback(task_tcb_s *task, void (*clean)(void *param), void *param)
{
	//�������callback����
	task -> clean = clean;
	//�������callback�����Ĳ���
	task -> clean_param = param;
}

//����ǿ��ɾ������
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
	
	//����task�������������
	if (task -> clean)
	{
		task -> clean(task -> clean_param);
	}
	
	//��ɾ�������Լ�������е���
	if (currentTask == task)
	{
		task_schedule();
	}
	
	task_exit_critical(status);
}

//��������ɾ������
//parameter
//task_tcb_s *task �� ϣ��ɾ��������
void rzyOS_task_request_delete(task_tcb_s *task)
{
	uint32_t status = task_enter_critical();
	
	//��ע����ɾ��״̬
	task -> request_delete_flag = 1;
	
	task_exit_critical(status);
}

//������������ɾ���ļ�⺯��
//return
//��������ɾ��״̬
uint8_t rzyOS_task_request_delete_check(void)
{
	uint8_t delete_status;
	
	uint32_t status = task_enter_critical();
	
	delete_status = currentTask -> request_delete_flag;
	
	task_exit_critical(status);
	
	return delete_status;
}

//��������ɾ������
void rzyOS_task_delete_self(void)
{
	uint32_t status = task_enter_critical();
	
	//���������� ��currentTask == task�� ��ǰ������Ȼ���ھ����б�
	rzyOS_task_ready_list_remove(currentTask);
	
	if (currentTask -> clean)
	{
		currentTask -> clean(currentTask -> clean_param);
	}
	
	task_schedule();
	
	task_exit_critical(status);
}

//��ȡ������Ϣ
void rzyOS_task_get_info(task_tcb_s *task, rzyOS_task_info_s *info)
{
	uint32_t *task_stack_end;

	uint32_t status = task_enter_critical();
	
	//���������Ϣ
	info -> delay_ticks = task -> delayTicks;
	info -> prio = task -> prio;
	info -> slice = task -> slice;
	info -> suspend_count = task -> suspend_count;
	info -> task_status = task -> task_status;

	//��ջͳ��
	info -> task_stack_size = task -> task_stack_size;
	//�������ͳ��
	info -> task_stack_free_size = 0;

	task_stack_end = task -> task_bottom_base;

	uint32_t *stack_top = task -> task_bottom_base + (task -> task_stack_size) / sizeof(tTaskStack);

	while ((0 == *task_stack_end ++) && (task_stack_end <= stack_top))
	{
		info -> task_stack_free_size ++;
	}
	info -> task_stack_free_size *= sizeof(tTaskStack);
	
	task_exit_critical(status);
}
