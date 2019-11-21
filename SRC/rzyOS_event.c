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
