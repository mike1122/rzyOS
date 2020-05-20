#include "rzyOS_schedule.h"
#include "ARMCM3.h"

//systick�ж���������
//modify system_ARMCM3.C to change XTAL and SYSTEM_CLOCK
//#define  XTAL            (12000000UL)     /* Oscillator frequency */
//#define  SYSTEM_CLOCK    (1 * XTAL)
void set_systick_period(uint32_t ms)
{
	SysTick -> LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick -> VAL = 0;
	SysTick -> CTRL = SysTick_CTRL_CLKSOURCE_Msk |
						SysTick_CTRL_TICKINT_Msk |		//�ж�ʹ��
						SysTick_CTRL_ENABLE_Msk;
}

//systick�жϺ���
void SysTick_Handler()
{
	task_systemtick_handler();
}

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
