#include "rzyOS.h"

task_tcb_s tcb_task1;
task_tcb_s tcb_task2;
task_tcb_s tcb_task3;
task_tcb_s tcb_task4;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];


int task1Flag;

//task1��clean���պ���
void task1_destory(void *param)
{
	task1Flag = *(uint8_t *)param;
}

void task1_entry(void *param)
{
	set_systick_period(10);

	uint8_t param_temp = 1;
	rzyOS_task_clean_callback(currentTask, task1_destory, &param_temp); //��task1��ע�����ٺ���
	for (;;)
	{
		task1Flag = 0;
		task_delay(1);
		task1Flag = 1;
		task_delay(1);
	}
}

int task2Flag;
void task2_entry(void *param)
{
	uint8_t task1_delete_flag = 0;
	
	for (;;)
	{
		task2Flag = 0;
		task_delay(1);
		task2Flag = 1;
		task_delay(1);
		
		if (0 == task1_delete_flag)
		{
			rzyOS_task_force_delete(&tcb_task1); //��task2��ǿ��ɾ��task1
			task1_delete_flag = 1;
		}
	}
}

int task3Flag;
void task3_entry(void *param)
{
	for (;;)
	{
		if (rzyOS_task_request_delete_check()) //��task3��ѯ�Ƿ���ɾ���Լ�������
		{
			task3Flag = 0;
			rzyOS_task_delete_self(); //ɾ��task3
		}
		
		task3Flag = 0;
		task_delay(1);
		task3Flag = 1;
		task_delay(1);
	}
}

int task4Flag;
void task4_entry(void *param)
{
	uint8_t task3_delete_flag = 0;
	
	for (;;)
	{
		task4Flag = 0;
		task_delay(1);
		task4Flag = 1;
		task_delay(1);
		
		if (0 == task3_delete_flag)
		{
			rzyOS_task_request_delete(&tcb_task3); //��task4������ɾ��task3
			task3_delete_flag = 1;
		}
	}
}


void rzyOS_app_init(void)
{
	task_init(&tcb_task1, task1_entry, (void *)0x11111111, 0, &task1Env[1024]);
	task_init(&tcb_task2, task2_entry, (void *)0x22222222, 1, &task2Env[1024]);
	task_init(&tcb_task3, task3_entry, (void *)0x33333333, 0, &task3Env[1024]);
	task_init(&tcb_task4, task4_entry, (void *)0x44444444, 1, &task4Env[1024]);
}
