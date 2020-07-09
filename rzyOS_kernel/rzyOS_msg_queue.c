#include "rzyOS_msg_queue.h"

//��Ϣ���г�ʼ��
//parameter
//rzyOS_msg_queue_s *rzyOS_msg_queue : Ҫʹ�õ���Ϣ���нṹ
//void **msg_buffer ����Ϣ����buffer��ַ
//uint32_t max_count ����Ϣ�������ռ�
void rzyOS_msg_queue_init(rzyOS_msg_queue_s *rzyOS_msg_queue, void **msg_buffer, uint32_t max_count)
{
	//�¼��ṹ��ʼ��
	rzyOS_event_init(&(rzyOS_msg_queue -> rzyOS_ecb), event_type_msg_queue);
	
	rzyOS_msg_queue -> msg_buffer = msg_buffer;
	rzyOS_msg_queue -> max_count = max_count;
	rzyOS_msg_queue -> count = 0;
	rzyOS_msg_queue -> read = 0;
	rzyOS_msg_queue -> write = 0;
}

//��Ϣ�ȴ�����������ģʽ��
//parameter
//rzyOS_msg_queue_s *rzyOS_msg_queue : ��Ϣ���нṹ
//void **msg �� ��Ϣָ��
//uint32_t wait_tick �� ��ʱ�ȴ�
uint32_t rzyOS_msg_queue_wait(rzyOS_msg_queue_s *rzyOS_msg_queue, void **msg, uint32_t wait_tick)
{
	uint32_t status = task_enter_critical();
	
	//���������Ƿ�����Ϣ
	if (rzyOS_msg_queue -> count > 0)
	{
		//��� > 0 �� ȡ����Ϣ
		rzyOS_msg_queue -> count --;
		//ȡ����Ϣָ��
		*msg = rzyOS_msg_queue -> msg_buffer[rzyOS_msg_queue -> read];
		//����������ƶ�
		rzyOS_msg_queue -> read ++;

		//�������Ѿ���ĩβ�ˣ� ��Ҫ��ͷ��
		if (rzyOS_msg_queue -> read >= rzyOS_msg_queue -> max_count)
		{
			rzyOS_msg_queue -> read = 0;
		}
		task_exit_critical(status);
		return error_no_error;
	}
	//������������Ϣ
	else
	{
		//��������뵽����ȴ�����
		rzyOS_event_wait(&(rzyOS_msg_queue -> rzyOS_ecb), currentTask, (void *)0, event_type_msg_queue, wait_tick);
		task_exit_critical(status);
		
		//�����л�
		task_schedule();
		
		//���л������󣬴�event_msg�л�ȡ��Ϣ
		*msg = currentTask -> event_msg;
		
		return currentTask -> wait_event_result;
	}
}

//��Ϣ�ȴ�������������ģʽ��
uint32_t rzyOS_msg_queue_nowait(rzyOS_msg_queue_s *rzyOS_msg_queue, void **msg)
{
	uint32_t status = task_enter_critical();
	
	//���������Ƿ�����Ϣ
	if (rzyOS_msg_queue -> count > 0)
	{
		//��� > 0 �� ȡ����Ϣ
		rzyOS_msg_queue -> count --;
		//ȡ����Ϣָ��
		*msg = rzyOS_msg_queue -> msg_buffer[rzyOS_msg_queue -> read];
		//����������ƶ�
		rzyOS_msg_queue -> read ++;

		//�������Ѿ���ĩβ�ˣ� ��Ҫ��ͷ��
		if (rzyOS_msg_queue -> read >= rzyOS_msg_queue -> max_count)
		{
			rzyOS_msg_queue -> read = 0;
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
uint32_t rzyOS_msg_queue_post(rzyOS_msg_queue_s *rzyOS_msg_queue, void *msg, uint32_t notify_option)
{
	uint32_t status = task_enter_critical();
	
	//ͳ���Ƿ��еȴ������� ���������ȴ�
	if (rzyOS_event_wait_count(&(rzyOS_msg_queue -> rzyOS_ecb)) > 0)
	{
		//�����¼����ƿ��е�һ��task����������Ϣָ��
		task_tcb_s *task_tcb = rzyOS_event_wakeup(&(rzyOS_msg_queue -> rzyOS_ecb), (void *)msg, error_no_error);
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
		if (rzyOS_msg_queue -> count >= rzyOS_msg_queue -> max_count)
		{
			task_exit_critical(status);

			return error_resource_full;
		}
		//��Ϣδ���������Ϣ����ֵ
		else
		{
			//��Ϣ����ģʽΪ��ǰ���룬���������ݸ��ǣ�������ǰд��
			if (notify_option & rzyOS_msg_queue_send_front)
			{
				//�����������ͷ��
				if (rzyOS_msg_queue -> read <= 0)
				{
					//�ö�����ָ��β��
					rzyOS_msg_queue -> read = rzyOS_msg_queue -> max_count -1;
				}
				//�������������ͷ��
				else
				{
					//��������ǰ�ƶ�
					rzyOS_msg_queue -> read --;
				}
				
				//������Ϣ
				rzyOS_msg_queue -> msg_buffer[rzyOS_msg_queue -> read] = msg;
			}
			//��Ϣ������
			else
			{
				//������Ϣ
				rzyOS_msg_queue -> msg_buffer[rzyOS_msg_queue -> write] = msg;
				rzyOS_msg_queue -> write ++;

				//д�����Ѿ��������ֵ����д�����ص�ͷ��
				if (rzyOS_msg_queue -> write >= rzyOS_msg_queue -> max_count)
				{
					rzyOS_msg_queue -> write = 0;
				}
			}
		}
		
		//��Ϣ����+1
		rzyOS_msg_queue -> count ++;
	}
	
	task_exit_critical(status);
	
	return error_no_error;
}

//��Ϣ���е����
void rzyOS_msg_queue_flush(rzyOS_msg_queue_s *rzyOS_msg_queue)
{
	uint32_t status = task_enter_critical();

	//���ȴ��¼�������Ϊ0, ��˵����������Ϣ��������ȴ���״̬�� ��Ҫ����
	if (rzyOS_event_wait_count(&(rzyOS_msg_queue -> rzyOS_ecb)) == 0)
	{
		rzyOS_msg_queue -> count = 0;
		rzyOS_msg_queue -> read = 0;
		rzyOS_msg_queue -> write = 0;
	}

	task_exit_critical(status);
}

//��Ϣ���е�ɾ��
//��rzyOS_event_remove_all()�����л��������뵽��������
uint32_t rzyOS_msg_queue_destory(rzyOS_msg_queue_s *rzyOS_msg_queue)
{
	uint32_t status = task_enter_critical();

	//�Ƴ��ȴ��б��е����� �����ظ���
	uint32_t count = rzyOS_event_remove_all(&(rzyOS_msg_queue -> rzyOS_ecb), (void *)0, error_delete);

	task_exit_critical(status);

	//���������ڵȴ��������һ���л�
	if (count > 0)
	{
		task_schedule();
	}

	return count;
}

//��Ϣ����״̬��ѯ
void rzyOS_msg_queue_get_info(rzyOS_msg_queue_s *rzyOS_msg_queue, rzyOS_msg_queue_info_s *rzyOS_msg_queue_info)
{
	uint32_t status = task_enter_critical();

	rzyOS_msg_queue_info -> count = rzyOS_msg_queue -> count;
	rzyOS_msg_queue_info -> max_count = rzyOS_msg_queue -> max_count;
	rzyOS_msg_queue_info -> task_count = rzyOS_event_wait_count(&(rzyOS_msg_queue -> rzyOS_ecb));

	task_exit_critical(status);
}
