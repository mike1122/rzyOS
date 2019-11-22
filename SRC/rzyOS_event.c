#include "rzyOS_event.h"
#include "rzyOS.h"

void rzyOS_event_init(rzyOS_ecb_s *ecb, rzyOS_event_type_e type)
{
	ecb -> type = event_type_unknow;
	list_init(&(ecb -> wait_list));
}

//�¼��ȴ�����
void rzyOS_event_wait(rzyOS_ecb_s *rzyOS_ecb, task_tcb_s *task_tcb, void *msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = task_enter_critical();
	
	task_tcb -> task_status |= state;
	task_tcb -> wait_event = rzyOS_ecb;
	task_tcb -> event_msg = msg;
	
	task_tcb -> wait_event_result = event_type_unknow;
	
	//�Ӿ����������Ƴ�
	task_remove_ready_list(task_tcb);
	//�����¼����ƿ�ĵȴ����е�β��,��Ϊ�Ȼ���¼���������ִ��,���Բ��뵽β��
	list_add_last(&(rzyOS_ecb -> wait_list), &(task_tcb -> link_node));
	
	if (timeout)
	{
		//����еȴ���ʱ, ����뵽��ʱ�ȴ�����
		delay_list_insert_time_node(task_tcb, timeout);
	}
	
	task_exit_critical(status);	
}

task_tcb_s *rzyOS_event_wakeup(rzyOS_ecb_s *rzyOS_ecb, void *msg, uint32_t result)
{
	node_t *node;
	task_tcb_s *task;
	uint32_t status = task_enter_critical();
	
	node = remove_list_first(&(rzyOS_ecb -> wait_list));
	
	if (node != (node_t *)0)
	{
		task = (task_tcb_s *)node_parent(node, task_tcb_s, link_node);
		task -> wait_event = (rzyOS_ecb_s *)0;
		task -> event_msg = msg;
		task -> wait_event_result = result;
		task -> task_status &= ~RZYOS_TASK_WAIT_MASK;
		
		if (task -> delayTicks != 0)
		{
			rzyOS_task_wakeup(task);
		}
		
		task_insert_ready_list(task);
	}
	
	task_exit_critical(status);
	
	return task;
}
