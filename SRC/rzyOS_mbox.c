#include "rzyOS_mbox.h"

//�����ʼ��
//parameter
//rzyOS_mbox_s *rzyOS_mbox : Ҫʹ�õ�����ṹ
//void **msg_buffer ������buffer��ַ
//uint32_t max_count ���������ռ�
void rzyOS_mbox_init(rzyOS_mbox_s *rzyOS_mbox, void **msg_buffer, uint32_t max_count)
{
	//�¼��ṹ��ʼ��
	rzyOS_event_init(&(rzyOS_mbox -> rzyOS_ecb), event_type_mbox);
	
	rzyOS_mbox -> msg_buffer = msg_buffer;
	rzyOS_mbox -> max_count = max_count;
	rzyOS_mbox -> count = 0;
	rzyOS_mbox -> read = 0;
	rzyOS_mbox -> write = 0;
}

//��Ϣ�ȴ�����������ģʽ��
uint32_t rzyOS_mbox_wait(rzyOS_mbox_s *rzyOS_mbox, void **msg, uint32_t wait_tick)
{
	uint32_t status = task_enter_critical();
	
	//���������Ƿ�����Ϣ
	if (rzyOS_mbox -> count > 0)
	{
		//��� > 0 �� ȡ����Ϣ
		rzyOS_mbox -> count --;
		//ȡ����Ϣָ��
		*msg = rzyOS_mbox -> msg_buffer[rzyOS_mbox -> read];
		//����������ƶ�
		rzyOS_mbox -> read ++;

		//�������Ѿ���ĩβ�ˣ� ��Ҫ��ͷ��
		if (rzyOS_mbox -> read >= rzyOS_mbox -> max_count)
		{
			rzyOS_mbox -> read = 0;
		}
		task_exit_critical(status);
		return error_no_error;
	}
	//������������Ϣ
	else
	{
		//��������뵽����ȴ�����
		rzyOS_event_wait(&(rzyOS_mbox -> rzyOS_ecb), currentTask, (void *)0, event_type_mbox, wait_tick);
		task_exit_critical(status);
		
		//�����л�
		task_schedule();
		
		//���л������󣬴�event_msg�л�ȡ��Ϣ
		*msg = currentTask -> event_msg;
		
		return currentTask -> wait_event_result;
	}
}

//��Ϣ�ȴ�������������ģʽ��
uint32_t rzyOS_mbox_nowait(rzyOS_mbox_s *rzyOS_mbox, void **msg)
{
	uint32_t status = task_enter_critical();
	
	//���������Ƿ�����Ϣ
	if (rzyOS_mbox -> count > 0)
	{
		//��� > 0 �� ȡ����Ϣ
		rzyOS_mbox -> count --;
		//ȡ����Ϣָ��
		*msg = rzyOS_mbox -> msg_buffer[rzyOS_mbox -> read];
		//����������ƶ�
		rzyOS_mbox -> read ++;

		//�������Ѿ���ĩβ�ˣ� ��Ҫ��ͷ��
		if (rzyOS_mbox -> read >= rzyOS_mbox -> max_count)
		{
			rzyOS_mbox -> read = 0;
		}

		task_exit_critical(status);

		return error_no_error;
	}
	//������������Ϣ�� �����������ȴ�����������
	else
	{
		task_exit_critical(status);
		
		//��������Դ
		return error_resource_unvaliable;
	}
}

//��Ϣ�ͷź���
uint32_t rzyOS_mbox_post(rzyOS_mbox_s *rzyOS_mbox, void *msg, uint32_t notify_option)
{
	uint32_t status = task_enter_critical();
	
	//ͳ���Ƿ��еȴ������� ���������ȴ�
	if (rzyOS_event_wait_count(&(rzyOS_mbox -> rzyOS_ecb)) > 0)
	{
		task_tcb_s *task_tcb = rzyOS_event_wakeup(&(rzyOS_mbox -> rzyOS_ecb), (void *)msg, error_no_error);
		if (task_tcb -> prio < currentTask -> prio)
		{
			task_schedule();
		}
	}
	else
	{
		if (rzyOS_mbox -> count >= rzyOS_mbox -> max_count)
		{
			task_exit_critical(status);
			return error_resource_full;
		}
		else
		{
			if (notify_option & rzyOS_mbox_send_front)
			{
				if (rzyOS_mbox -> read <= 0)
				{
					rzyOS_mbox -> read = rzyOS_mbox -> max_count -1;
				}
				else
				{
					rzyOS_mbox -> read --;
				}
				
				rzyOS_mbox -> msg_buffer[rzyOS_mbox -> read] = msg;
			}
			else
			{
				rzyOS_mbox -> msg_buffer[rzyOS_mbox -> write] = msg;
				rzyOS_mbox -> write ++;
				if (rzyOS_mbox -> write >= rzyOS_mbox -> max_count)
				{
					rzyOS_mbox -> write = 0;
				}
			}
		}
		
		rzyOS_mbox -> count ++;
	}
	
	task_exit_critical(status);
	
	return error_no_error;
}
