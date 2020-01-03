#include "rzyOS_event.h"
#include "rzyOS.h"

//�¼���ʼ��
//parameter
//rzyOS_ecb_s *ecb �� ����ʼ����������ƿ�
//rzyOS_event_type_e type �� ��ʼ�����¼�����
void rzyOS_event_init(rzyOS_ecb_s *ecb, rzyOS_event_type_e type)
{
	//�¼�����
	ecb -> type = type;
	//��ʼ���¼��ȴ��б�
	list_init(&(ecb -> wait_list));
}

//�¼��ȴ���������Ϊ��������� �������ȴ��� ���ź����ȴ��������������ȣ�
//��������뵽�¼��ĵȴ�����
//parameter
//rzyOS_ecb_s *rzyOS_ecb �� ����Ҫ������¼����ƿ�
//task_tcb_s *task_tcb �� Ҫ���������
//void *msg �� ��Ϣ��ַ
//uint32_t state �� ����״̬
//uint32_t timeout �� ��ʱ�ȴ�
void rzyOS_event_wait(rzyOS_ecb_s *rzyOS_ecb, task_tcb_s *task_tcb, void *msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = task_enter_critical();
	
	task_tcb -> task_status |= state;
	//��������ȴ����¼����ƿ�
	task_tcb -> wait_event = rzyOS_ecb;
	task_tcb -> event_msg = msg;
	
	//��λ�¼��ȴ����
	task_tcb -> wait_event_result = error_no_error;
	
	//�Ӿ����������Ƴ�
	task_remove_ready_list(task_tcb);
	//�����¼����ƿ�ĵȴ����е�β��, ��Ϊ�Ȼ���¼���������ִ��, ���Բ��뵽β��
	list_add_last(&(rzyOS_ecb -> wait_list), &(task_tcb -> link_node));
	
	//����г�ʱ�ȴ��� ���������ʱ��ȴ�����Ҳ����ʱ����
	//����ʱ�ȴ���Ч�� ������ʱ��������а�������¼��ȴ������Ƴ�
	if (timeout)
	{
		//����еȴ���ʱ, ����뵽��ʱ�ȴ�����
		delay_list_insert_time_node(task_tcb, timeout);
	}
	
	task_exit_critical(status);
}

//����֪ͨ����¼����ƿ�
//��������¼��б�ĵ�һ������
task_tcb_s *rzyOS_event_wakeup(rzyOS_ecb_s *rzyOS_ecb, void *msg, uint32_t result)
{
	node_t *node;
	task_tcb_s *task;

	uint32_t status = task_enter_critical();
	
	//���¼��ȴ����еĵ�һ��������б�ɾ���� �������¼��ȴ����нڵ�ָ��
	node = remove_list_first(&(rzyOS_ecb -> wait_list));
	
	if (node != (node_t *)0)
	{
		//������Ӧ��TCBֵ
		task = (task_tcb_s *)node_parent(node, task_tcb_s, link_node);
		task -> wait_event = (rzyOS_ecb_s *)0;
		task -> event_msg = msg;
		task -> wait_event_result = result;
		task -> task_status &= ~RZYOS_TASK_WAIT_MASK;
		
		//�������˳�ʱ�ȴ�
		if (task -> delayTicks != 0)
		{
			//��ǿ�ƴ���ʱ�����л���, �Ա��¼�����, ��ʱ��Ӧ
			rzyOS_task_delay_list_remove(task);
		}
		
		//��������뵽��������, �ȴ�����
		task_insert_ready_list(task);
	}
	
	task_exit_critical(status);
	
	return task;
}

//��ָ����������¼����ƿ�ǿ���Ƴ�
void rzyOS_event_remove(task_tcb_s *task_tcb, void *msg, uint32_t result)
{
	uint32_t status = task_enter_critical();

	//���¼��ȴ��б����Ƴ�ָ��������
	list_remove_pos_node(&(task_tcb -> wait_event -> wait_list), &(task_tcb -> link_node));
	//���ó�0 , ���ٵȴ��¼�
	task_tcb -> wait_event = (rzyOS_ecb_s *)0;
	task_tcb -> event_msg = msg;
	task_tcb -> wait_event_result = result;
	task_tcb -> task_status &= ~RZYOS_TASK_WAIT_MASK;

	task_exit_critical(status);

	//�˺�������systemtick�е���, ����systick�ж��л���, ���Բ�������ǿ�ƻ��Ѵ���
}

//�Ƴ��¼����������е����ڵȴ��¼�������
//�����Ƴ��ĸ���
uint32_t rzyOS_event_remove_all(rzyOS_ecb_s *rzyOS_ecb, void *msg, uint32_t result)
{
	uint32_t element_count = 0;
	task_tcb_s *task;
	node_t *node;

	uint32_t status = task_enter_critical();

	//ͳ���¼��еȴ������е�������
	element_count = list_count(&(rzyOS_ecb -> wait_list));

	while ((node = remove_list_first(&(rzyOS_ecb -> wait_list))) != (node_t *)0)
	{
		task = (task_tcb_s *)node_parent(node, task_tcb_s, link_node);
		//���ó�0 , ���ٵȴ��¼�
		task -> wait_event = (rzyOS_ecb_s *)0;
		task -> event_msg = msg;
		task -> wait_event_result = result;
		task -> task_status &= ~RZYOS_TASK_WAIT_MASK;

		if (task -> delayTicks != 0)
		{
			//�������ʱ, ��ǿ�ƴ���ʱ�����л���, �Ա��¼�����, ��ʱ��Ӧ
			rzyOS_task_delay_list_remove(task);
		}

		task_insert_ready_list(task);
	}

	task_exit_critical(status);

	//�����Ƴ��ĸ���
	return element_count;
}

//ͳ���¼��ȴ����������������
uint32_t rzyOS_event_wait_count(rzyOS_ecb_s *rzyOS_ecb)
{
	uint32_t count = 0;
	
	uint32_t status = task_enter_critical();
	
	//ͳ���¼��ȴ��������������
	count = list_count(&(rzyOS_ecb -> wait_list));
	
	task_exit_critical(status);
	
	return count;
}
