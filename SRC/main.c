#include "rzyOS.h"
#include "ARMCM3.h"

tTask *currentTask;
tTask *nextTask;
tTask *idleTask;
tTask *taskTable[2];

uint8_t schedLockCount;


void tTaskInit(tTask *task, void (*entry)(void *), void *param, tTaskStack *stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x03;
	*(--stack) = (unsigned long)0x02;
	*(--stack) = (unsigned long)0x01;
	*(--stack) = (unsigned long)param;
	
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x09;
	*(--stack) = (unsigned long)0x08;
	*(--stack) = (unsigned long)0x07;
	*(--stack) = (unsigned long)0x06;
	*(--stack) = (unsigned long)0x05;
	*(--stack) = (unsigned long)0x04;
	
	task -> stack = stack;
	task -> delayTicks = 0;
}

void tTaskSched(void)
{
	uint32_t status = tTaskEnterCritical();
	
	if (schedLockCount > 0)
	{
		tTaskExitCritical(status);
		return ;
	}
	if (currentTask == idleTask)
	{
		if (taskTable[0] -> delayTicks == 0)
		{
			nextTask = taskTable[0];
		}
		else if (taskTable[1] -> delayTicks == 0)
		{
			nextTask = taskTable[1];
		}
		else
		{
			tTaskExitCritical(status);
			return ;
		}
	}
	else
	{
		if (currentTask == taskTable[0])
		{
			if (taskTable[1] -> delayTicks == 0)
			{
				nextTask = taskTable[1];
			}
			else if (currentTask -> delayTicks != 0)
			{
				nextTask = idleTask;
			}
			else
			{
				tTaskExitCritical(status);
				return ;
			}
		}
		else if (currentTask == taskTable[1])
		{
			if (taskTable[0] -> delayTicks == 0)
			{
				nextTask = taskTable[0];
			}
			else if (currentTask -> delayTicks != 0)
			{
				nextTask = idleTask;
			}
			else
			{
				tTaskExitCritical(status);
				return ;
			}
		}
	}
	
	tTaskExitCritical(status);
	
	tTaskSwitch();
}

void tTaskSchedInit(void)
{
	schedLockCount = 0;
}

void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();
	
	if (schedLockCount < 0xff)
	{
		schedLockCount ++;
	}
	
	tTaskExitCritical(status);
}

void tTaskschedEnable(void)
{
	uint32_t status = tTaskEnterCritical();
	
	if (schedLockCount > 0)
	{
		schedLockCount --;
		if (0 == schedLockCount)
		{
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

void tTasksystemTickHandler()
{
	uint32_t status = tTaskEnterCritical();
	
	int i;
	for (i = 0; i < 2; i ++)
	{
		if (taskTable[i] -> delayTicks > 0)
		{
			(taskTable[i] -> delayTicks) --;
		}
	}
	
	tTaskExitCritical(status);
	tTaskSched();
}

void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();
	
	currentTask -> delayTicks = delay;
	
	tTaskExitCritical(status);
	tTaskSched();
}

//modify system_ARMCM3.C to change XTAL and SYSTEM_CLOCK
//#define  XTAL            (12000000UL)     /* Oscillator frequency */
//#define  SYSTEM_CLOCK    (1 * XTAL)
void SetSysTickPeriod(uint32_t ms)
{
	SysTick -> LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick -> VAL = 0;
	SysTick -> CTRL = SysTick_CTRL_CLKSOURCE_Msk |
						SysTick_CTRL_TICKINT_Msk |		//�ж�ʹ��
						SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler()
{
	tTasksystemTickHandler();
}

void delay(int count)
{
	while(-- count > 0);
}

int shareCount;

int task1Flag;
void task1Entry(void *param)
{
	SetSysTickPeriod(10);
	for (;;)
	{
		int var;
		
		tTaskSchedDisable();
		var = shareCount;
		
		task1Flag = 0;
		tTaskDelay(1);
		
		var ++;
		shareCount = var;
		tTaskschedEnable();
		
		task1Flag = 1;
		tTaskDelay(1);
	}
}

int task2Flag;
void task2Entry(void *param)
{
	for (;;)
	{
		tTaskSchedDisable();
		shareCount ++;
		tTaskschedEnable();
		
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
	}
}

tTask tTask1;
tTask tTask2;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];
void idleTaskEntry(void *param)
{
	for (;;)
	{
	}
}

int main()
{
	tTaskSchedInit();
	
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, &task2Env[1024]);
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, &idleTaskEnv[1024]);
	idleTask = &tTaskIdle;
	
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	
	nextTask = taskTable[0];
	
	tTaskRunFirst();
	
	return 0;
}
