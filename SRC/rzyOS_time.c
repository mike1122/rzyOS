#include "rzyOS.h"
#include "rzyOS_semaphore.h"

static list_t rzyOS_high_wqueue_list;
static list_t rzyOS_low_wqueue_list;

static rzyOS_sem_s rzyOS_wqueue_protect_sem;
static rzyOS_sem_s rzyOS_wqueue_tick_sem;

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



void rzyOS_wqueue_task_init(void)
{
	list_init(&rzyOS_high_wqueue_list);
	list_init(&rzyOS_low_wqueue_list);

	rzyOS_sem_init(&rzyOS_wqueue_protect_sem, 1, 1);
	rzyOS_sem_init(&rzyOS_wqueue_tick_sem, 0, 0);
}
