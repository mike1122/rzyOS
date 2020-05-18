#include "rzyOS_schedule.h"

//task�е���ʱ����,ʹ����ʱ���н��д���
//param: delay--systick���ڼ���
void task_delay(uint32_t delay)
{
	uint32_t status = task_enter_critical();

	//����ǰ���������ʱ�ȴ��б���������ʱʱ��
	delay_list_insert_time_node(currentTask, delay);

	//��һ״̬���ھ����б���ǰ�Ѳ�����ʱ�ȴ��б����ԴӾ����б����Ƴ�
	task_remove_ready_list(currentTask);

	task_exit_critical(status);

	task_schedule();
}
