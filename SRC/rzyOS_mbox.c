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
//parameter
//rzyOS_mbox_s *rzyOS_mbox : ����ṹ
//void **msg �� ��Ϣָ��
//uint32_t wait_tick �� ��ʱ�ȴ�
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

//postһ����Ϣ
uint32_t rzyOS_mbox_post(rzyOS_mbox_s *rzyOS_mbox, void *msg, uint32_t notify_option)
{
	uint32_t status = task_enter_critical();
	
	//ͳ���Ƿ��еȴ������� ���������ȴ�
	if (rzyOS_event_wait_count(&(rzyOS_mbox -> rzyOS_ecb)) > 0)
	{
		//�����¼����ƿ��е�һ��task����������Ϣָ��
		task_tcb_s *task_tcb = rzyOS_event_wakeup(&(rzyOS_mbox -> rzyOS_ecb), (void *)msg, error_no_error);
		//�ж����ȼ����Ƿ���Ҫ����
		if (task_tcb -> prio < currentTask -> prio)
		{
			task_schedule();
		}
	}
	//û�еȴ����񣬽�����Ϣ����
	else
	{
		//��Ϣ�Ѿ������������Ϣ����ֵ
		if (rzyOS_mbox -> count >= rzyOS_mbox -> max_count)
		{
			task_exit_critical(status);

			return error_resource_full;
		}
		//��Ϣδ���������Ϣ����ֵ
		else
		{
			//��Ϣ����ģʽΪ��ǰ���룬���������ݸ��ǣ�������ǰд��
			if (notify_option & rzyOS_mbox_send_front)
			{
				//�����������ͷ��
				if (rzyOS_mbox -> read <= 0)
				{
					//�ö�����ָ��β��
					rzyOS_mbox -> read = rzyOS_mbox -> max_count -1;
				}
				//�������������ͷ��
				else
				{
					//��������ǰ�ƶ�
					rzyOS_mbox -> read --;
				}
				
				//������Ϣ
				rzyOS_mbox -> msg_buffer[rzyOS_mbox -> read] = msg;
			}
			//��Ϣ������
			else
			{
				//������Ϣ
				rzyOS_mbox -> msg_buffer[rzyOS_mbox -> write] = msg;
				rzyOS_mbox -> write ++;

				//д�����Ѿ��������ֵ����д�����ص�ͷ��
				if (rzyOS_mbox -> write >= rzyOS_mbox -> max_count)
				{
					rzyOS_mbox -> write = 0;
				}
			}
		}
		
		//��Ϣ����+1
		rzyOS_mbox -> count ++;
	}
	
	task_exit_critical(status);
	
	return error_no_error;
}

//��������
void rzyOS_mbox_flush(rzyOS_mbox_s *rzyOS_mbox)
{
	uint32_t status = task_enter_critical();

	//���ȴ��¼�������Ϊ0, ��˵����������Ϣ��������ȴ���״̬�� ��Ҫ����
	if (rzyOS_event_wait_count(&(rzyOS_mbox -> rzyOS_ecb)) == 0)
	{
		rzyOS_mbox = count;
		rzyOS_mbox = read;
		rzyOS_mbox = write;
	}

	task_exit_critical(status);
}

//�����ɾ��
//��rzyOS_event_remove_all()�����л��������뵽��������
uint32_t rzyOS_mbox_destory(rzyOS_mbox_s *rzyOS_mbox)
{
	uint32_t status = task_enter_critical();

	//�Ƴ��ȴ��б��е����� �����ظ���
	uint32_t count = rzyOS_event_remove_all(&(rzyOS_mbox -> rzyOS_ecb), (void *)0, error_delete);

	task_exit_critical(status);

	//���������ڵȴ��������һ���л�
	if (count > 0)
	{
		task_schedule();
	}

	return count;
}

//����״̬��ѯ
void rzyOS_mbox_get_info(rzyOS_mbox_s *rzyOS_mbox, rzyOS_mbox_info_s *rzyOS_mbox_info)
{
	uint32_t status = task_enter_critical();

	rzyOS_mbox_info -> count = rzyOS_mbox -> count;
	rzyOS_mbox_info -> max_count = rzyOS_mbox -> max_count;
	rzyOS_mbox_info -> task_count = rzyOS_mbox -> rzyOS_event_wait_count(&(rzyOS_mbox -> rzyOS_ecb));

	task_exit_critical(status);
}
