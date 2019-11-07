#include "rzyOS.h"

//task�е���ʱ����,ʹ����ʱ���н��д���
//param: delay--systick���ڼ���
void task_delay(uint32_t delay)
{
	uint32_t status = task_enter_critical();

	delay_list_insert_time_node(currentTask, delay);

	task_remove_ready_list(currentTask);

	task_exit_critical(status);

	task_schedule();
}
