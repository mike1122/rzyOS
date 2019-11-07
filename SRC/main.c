#include "rzyOS.h"
#include "ARMCM3.h"

task_tcb_s *currentTask;
task_tcb_s *nextTask;
task_tcb_s *idleTask;

bitmap_s bitmap_taskprio;
list_t task_ready_table[RZYOS_PRIO_COUNT];

uint8_t schedLockCount;

list_t task_delay_list;


//��λͼ���ҵ�������������ȼ�
//���վ�����������ȼ��ھ��������б��л�ȡ��һ������ڵ�
//�ӵ�һ������ڵ�õ���Ӧ��task��TCB
task_tcb_s *task_highest_ready(void)
{
	uint32_t highest_prio = bitmap_get_first_set(&bitmap_taskprio);
	node_t *node = list_first_node(&task_ready_table[highest_prio]);
	return node_parent(node, task_tcb_s, link_node);
}

//�������ȼ�,�Ѿ��������������ڵ�����������list��
void task_insert_ready_list(task_tcb_s *task_tcb)
{
	//������ڵ���ӵ����ȼ�������
	list_add_first(&task_ready_table[task_tcb -> prio], &(task_tcb -> link_node));
	bitmap_set(&bitmap_taskprio, task_tcb -> prio);
}

//��taskʹ����ʱ����,����ô˺���
//��Ϊ���������µ���ʱ, ����Ҫ�ھ���list��ɾ����ǰ���������ڵ�
void task_remove_ready_list(task_tcb_s *task_tcb)
{
	list_remove_pos_node(&task_ready_table[task_tcb -> prio], &(task_tcb -> link_node));
	
	if (0 == list_count(&task_ready_table[task_tcb -> prio]))
	{
		bitmap_clean(&bitmap_taskprio, task_tcb -> prio);
	}
}

//task����, �л���������ȼ���,��ʱ�����task
void task_schedule(void)
{
	task_tcb_s *tempTask;
	uint32_t status = task_enter_critical();
	
	if (schedLockCount > 0)
	{
		task_exit_critical(status);
		return ;
	}
	
	tempTask = task_highest_ready();
	if (tempTask != currentTask)
	{
		nextTask = tempTask;
		task_switch(); //����pendSV
	}
	
	task_exit_critical(status);
}

//��ʼ��λͼ
//��ʼ�����������list����(���鰴�����ȼ�����)
void task_schedule_init(void)
{
	int i;
	
	schedLockCount = 0;
	bitmap_init(&bitmap_taskprio);
	for (i = 0; i < RZYOS_PRIO_COUNT; i ++)
	{
		list_init(&task_ready_table[i]);
	}
}

void task_schedule_disable(void)
{
	uint32_t status = task_enter_critical();
	
	if (schedLockCount < 0xff)
	{
		schedLockCount ++;
	}
	
	task_exit_critical(status);
}

void task_schedule_enable(void)
{
	uint32_t status = task_enter_critical();
	
	if (schedLockCount > 0)
	{
		schedLockCount --;
		if (0 == schedLockCount)
		{
			task_schedule();
		}
	}
	
	task_exit_critical(status);
}

//��ʼ����ʱ�б�list
void task_delay_list_init(void)
{
	list_init(&task_delay_list);
}

//��taskʹ����ʱ����,����ô˺���
//�ڵ�ǰ�����TCB��д����Ҫ����ʱ
//����ʱlist�м��뵱ǰ�������ʱ�ڵ�
void delay_list_insert_time_node(task_tcb_s *task_tcb, uint32_t ticks)
{
	task_tcb -> delayTicks = ticks;
	list_add_first(&task_delay_list, &(task_tcb -> delay_node));
	task_tcb -> ready_status |= RZYOS_TASK_STATUS_DELAY;
}

//����ʱ������ɾ��delay�Ѿ�Ϊ0����ʱ�ڵ�
void delay_list_remove_time_node(task_tcb_s *task_tcb)
{
	list_remove_pos_node(&task_delay_list, &(task_tcb -> delay_node));
	task_tcb -> ready_status &= ~RZYOS_TASK_STATUS_DELAY;
}

//systick�жϵ��ô˺���
void task_systemtick_handler(void)
{
	node_t *node;
	
	uint32_t status = task_enter_critical();
	
	for (node = task_delay_list.head_node.next_node; node != &(task_delay_list.head_node); node = node -> next_node)
	{
		task_tcb_s *task_tcb =  (task_tcb_s *)node_parent(node, task_tcb_s, delay_node);
		task_tcb -> delayTicks --;
		if (0 == task_tcb -> delayTicks)
		{
			delay_list_remove_time_node(task_tcb);
			
			task_insert_ready_list(task_tcb);
		}
	}
	
	//ʱ��Ƭ��ת
	//��Ϊ���ȵ�nextTask�������ȼ��б�ĵص�һ���ڵ�
	//���Ի���ʱ��Ƭ����Ҫ�Ե�һ���ڵ����,Ҳ����currentTask
	currentTask -> slice --;
	//�жϵ�ǰ����ʱ��Ƭ�Ƿ�����
	if (0 == currentTask -> slice)
	{
		//�������, �ҵ�ǰ�����ͬ���ȼ�����list��������, ���л���ת
		if (list_count(&task_ready_table[currentTask -> prio]) > 0)
		{
			//�Ƴ�ͬ���ȼ�����list�е�����ڵ�, �����뵽���һ���ڵ�, ��ɻ���ͬ���ȼ���ʱ��Ƭ����
			remove_list_first(&task_ready_table[currentTask -> prio]);
			list_add_last(&task_ready_table[currentTask -> prio], &(currentTask -> link_node));
			
			//����ʱ��Ƭ
			currentTask -> slice = RZYOS_SLICE_MAX;
		}
	}
	
	task_exit_critical(status);
	
	task_schedule();
}

void delay(int count)
{
	while(-- count > 0);
}


task_tcb_s tcb_task_idle;
tTaskStack idleTaskEnv[RZYOS_IDLETASK_STACK_SIZE];

void idle_task_entry(void *param)
{
	for (;;)
	{
		
	}
}

int main()
{
	task_schedule_init();
	
	task_delay_list_init();
	
	rzyOS_app_init();
	
	task_init(&tcb_task_idle, idle_task_entry, (void *)0, RZYOS_PRIO_COUNT - 1, &idleTaskEnv[RZYOS_IDLETASK_STACK_SIZE]);
	idleTask = &tcb_task_idle;
	
	
	nextTask = task_highest_ready();
	
	task_run_first();
	
	return 0;
}
