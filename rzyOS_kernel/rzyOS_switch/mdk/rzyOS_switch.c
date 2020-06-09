#include "rzyOS_schedule.h"
#include "rzyOSarch.h"
#include "r_string.h"


//�жϿ�������ַ
#define NVIC_INT_CTRL	0xe000ed04
//����PendSV
#define	NVIC_PENDSVSET	0X10000000
//PendSV���ȼ����Ƶ�ַ
#define	NVIC_SYSPRI2	0xe000ed22
//PendSV����Ϊ�������ֵ255
#define	NVIC_PENDSV_PRI	0x000000ff



#define MEM32(addr)	*(volatile unsigned long *)(addr)
#define MEM8(addr)	*(volatile unsigned char *)(addr)

//�����ٽ�������
uint32_t task_enter_critical(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	return primask;
}

//�˳��ٽ�������
void task_exit_critical(uint32_t status)
{
	__set_PRIMASK(status);
}


#ifdef ARMCM3_SIM
//pendsv�жϷ�����
__asm void PendSV_Handler(void)
{
	IMPORT currentTask					// IMPORT������溯������������������൱��C�е�extern�ؼ��֣�ָ����Щȫ�ַ�����������Դ�ļ��ж����
	IMPORT nextTask
	
	MRS R0, PSP							// MRS <gp_reg>, <special_reg> ;�����⹦�ܼĴ�����ֵ��ͨ�üĴ���
	CBZ R0, PendSVHander_nosave			// �Ƚϣ�������Ϊ 0 ��ת��
	STMDB R0!, {R4-R11}					// �洢R4-R11�� R0 ��ַ����ÿ��һ���ֺ�(!) Rd ����һ�Σ�32λ���
	LDR R1, =currentTask				// ȡcurrentTask����������ŵĵ�ַд��R1��ע�⣬����ȡcurrentTask��ֵ
	LDR R1, [R1]						// R1= *R1
	STR R0, [R1]						// ��R0�洢��R1��ַ��
	
PendSVHander_nosave
	LDR R0, =currentTask
	LDR R1, =nextTask
	LDR R2, [R1]
	STR R2, [R0]
	
	LDR R0, [R2]
	LDMIA R0!, {R4-R11}					// �� R0 ����ȡ����֡�д��R4-R11��, ÿ��һ���ֺ� R0 ����һ�Σ�16λ���
	
	MSR PSP, R0							// MRS <special_reg>,  <gp_reg> ;�洢gp_reg��ֵ�����⹦�ܼĴ���
	ORR LR, LR, #0X04					// ORR��λ�� ���ʹ��PSP
	BX LR								// ��󷵻أ���ʱ����ͻ�Ӷ�ջ��ȡ��LRֵ���ָ����ϴ����е�λ
}
#endif


#if defined(STM32F40_41xxx) || defined(STM32F40XX)
//pendsv�жϷ�����
__asm void PendSV_Handler(void)
{
	IMPORT saveAndLoadStackAddr

	// �л���һ������ʱ,����������PSP=MSP�����������STMDB����ὫR4~R11
	// ���浽ϵͳ����ʱĬ�ϵ�MSP��ջ�У�������ĳ������
	MRS     R0, PSP

	STMDB   R0!, {R4-R11}				// ��R4~R11���浽��ǰ����ջ��Ҳ����PSPָ��Ķ�ջ
	VSTMDB  R0!, {S16-S31}				// ���渡��S16-31
	BL      saveAndLoadStackAddr		// ���ú���������ͨ��R0���ݣ�����ֵҲͨ��R0���� 
	VLDMIA  R0!, {S16-S31}				// �ָ�����S16-31
	LDMIA   R0!, {R4-R11}				// ����һ����Ķ�ջ�У��ָ�R4~R11

	MSR     PSP, R0
	MOV     LR, #0xFFFFFFED				// ָ�������쳣ʱʹ��PSP��ע�⣬��ʱLR���ǳ��򷵻ص�ַ
	BX      LR
}

uint32_t saveAndLoadStackAddr(uint32_t stackAddr)
{
	//��һ���л�ʱ�� ��ǰ����tcbΪ0�� ���Բ��ᱣ��
	if (currentTask != (task_tcb_s *)0)
	{
		//�����ջ��ַ
		currentTask -> stack = (uint32_t *)stackAddr;
	}

	currentTask = nextTask;

	//ȡ��һ�����ջ��ַ
	return (uint32_t)currentTask -> stack;
}
#endif


//��MSPת��PSP,����pendsv���ȼ�
//change MSP to PSP , then setup pendSV priority and trigger pendSV
void task_run_first(void)
{
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

//����pendsv�ж�
//trigger pendSV
void task_switch(void)
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

#ifdef ARMCM3_SIM
//task��ʼ��
void task_init(task_tcb_s *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack_bottom, uint32_t task_size)
{
	//����ջ��(�ߵ�ַ)
	uint32_t *stack_top;
	//ջ�׸�ֵ(�͵�ַ)
	task -> task_bottom_base = stack_bottom;
	//ջ��С
	task -> task_stack_size = task_size;
	//ջ����
	memset(stack_bottom, 0, task_size);

	//����ջ����ַ
	stack_top = stack_bottom + task_size / sizeof(tTaskStack);

	//��ʼ�������ջ����Ӧ��ͨ�üĴ���
	*(--stack_top) = (unsigned long)(1 << 24);
	*(--stack_top) = (unsigned long)entry;
	*(--stack_top) = (unsigned long)0x14;
	*(--stack_top) = (unsigned long)0x12;
	*(--stack_top) = (unsigned long)0x03;
	*(--stack_top) = (unsigned long)0x02;
	*(--stack_top) = (unsigned long)0x01;
	*(--stack_top) = (unsigned long)param;
	
	*(--stack_top) = (unsigned long)0x11;
	*(--stack_top) = (unsigned long)0x10;
	*(--stack_top) = (unsigned long)0x09;
	*(--stack_top) = (unsigned long)0x08;
	*(--stack_top) = (unsigned long)0x07;
	*(--stack_top) = (unsigned long)0x06;
	*(--stack_top) = (unsigned long)0x05;
	*(--stack_top) = (unsigned long)0x04;
	
	//�����ջ��ָ̬�봫��
	task -> stack = stack_top;
	//��ʼ������tcb
	node_init(&(task -> link_node));
	task -> delayTicks = 0;
	task -> prio = prio;
	task -> task_status = RZYOS_TASK_STATUS_READY;
	task -> slice = RZYOS_SLICE_MAX;
	node_init(&(task -> delay_node));
	task -> suspend_count = 0;
	task -> clean = (void (*) (void *))0;
	task -> clean_param = (void *)0;
	task -> request_delete_flag = 0;
	
	//��ʼ���������������������еȴ�����
	task_insert_ready_list(task);
}
#endif


#if defined(STM32F40_41xxx) || defined(STM32F40XX)
//task��ʼ��
void task_init(task_tcb_s *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack_bottom, uint32_t task_size)
{
	//����ջ��(�ߵ�ַ)
	uint32_t *stack_top;
	//ջ�׸�ֵ(�͵�ַ)
	task -> task_bottom_base = stack_bottom;
	//ջ��С
	task -> task_stack_size = task_size;
	//ջ����
	r_memset(stack_bottom, 0, task_size);

	//����ջ����ַ
	stack_top = stack_bottom + task_size / sizeof(tTaskStack);

	*(--stack_top) = (unsigned long)(0);					// ��,δ��
	*(--stack_top) = (unsigned long)(0);					// FPSCR
	*(--stack_top) = (unsigned long)(0x15);					// S15
	*(--stack_top) = (unsigned long)(0x14);					// S14
	*(--stack_top) = (unsigned long)(0x13);					// S13
	*(--stack_top) = (unsigned long)(0x12);					// S12
	*(--stack_top) = (unsigned long)(0x11);					// S11
	*(--stack_top) = (unsigned long)(0x10);					// S10
	*(--stack_top) = (unsigned long)(0x9);					// S9
	*(--stack_top) = (unsigned long)(0x8);					// S8
	*(--stack_top) = (unsigned long)(0x7);					// S7
	*(--stack_top) = (unsigned long)(0x6);					// S6
	*(--stack_top) = (unsigned long)(0x5);					// S5
	*(--stack_top) = (unsigned long)(0x4);					// S4
	*(--stack_top) = (unsigned long)(0x3);					// S3
	*(--stack_top) = (unsigned long)(0x2);					// S2
	*(--stack_top) = (unsigned long)(0x1);					// S1
	*(--stack_top) = (unsigned long)(0x0);					// S0

	*(--stack_top) = (unsigned long)(1 << 24);				// XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
	*(--stack_top) = (unsigned long)entry;					// �������ڵ�ַ
	*(--stack_top) = (unsigned long)0x14;					// R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
	*(--stack_top) = (unsigned long)0x12;					// R12, δ��
	*(--stack_top) = (unsigned long)0x3;					// R3, δ��
	*(--stack_top) = (unsigned long)0x2;					// R2, δ��
	*(--stack_top) = (unsigned long)0x1;					// R1, δ��
	*(--stack_top) = (unsigned long)param;					// R0 = param, �����������ں���
	*(--stack_top) = (unsigned long)0x11;					// R11, δ��
	*(--stack_top) = (unsigned long)0x10;					// R10, δ��
	*(--stack_top) = (unsigned long)0x9;					// R9, δ��
	*(--stack_top) = (unsigned long)0x8;					// R8, δ��
	*(--stack_top) = (unsigned long)0x7;					// R7, δ��
	*(--stack_top) = (unsigned long)0x6;					// R6, δ��
	*(--stack_top) = (unsigned long)0x5;					// R5, δ��
	*(--stack_top) = (unsigned long)0x4;					// R4, δ��

	*(--stack_top) = (unsigned long)(0x31);					// S31
	*(--stack_top) = (unsigned long)(0x30);					// S30
	*(--stack_top) = (unsigned long)(0x29);					// S29
	*(--stack_top) = (unsigned long)(0x28);					// S28
	*(--stack_top) = (unsigned long)(0x27);					// S27
	*(--stack_top) = (unsigned long)(0x26);					// S26
	*(--stack_top) = (unsigned long)(0x25);					// S25
	*(--stack_top) = (unsigned long)(0x24);					// S24
	*(--stack_top) = (unsigned long)(0x23);					// S23
	*(--stack_top) = (unsigned long)(0x22);					// S22
	*(--stack_top) = (unsigned long)(0x21);					// S21
	*(--stack_top) = (unsigned long)(0x20);					// S20
	*(--stack_top) = (unsigned long)(0x19);					// S19
	*(--stack_top) = (unsigned long)(0x18);					// S18
	*(--stack_top) = (unsigned long)(0x17);					// S17
	*(--stack_top) = (unsigned long)(0x16);					// S16

	
	//�����ջ��ָ̬�봫��
	task -> stack = stack_top;
	//��ʼ������tcb
	node_init(&(task -> link_node));
	task -> delayTicks = 0;
	task -> prio = prio;
	task -> task_status = RZYOS_TASK_STATUS_READY;
	task -> slice = RZYOS_SLICE_MAX;
	node_init(&(task -> delay_node));
	task -> suspend_count = 0;
	task -> clean = (void (*) (void *))0;
	task -> clean_param = (void *)0;
	task -> request_delete_flag = 0;
	
	//��ʼ�����������������ʱ���еȴ�����
	task_insert_ready_list(task);
}
#endif

void rzyOS_start(void)
{
	nextTask = task_highest_ready();

	SysTick -> CTRL |= SysTick_CTRL_ENABLE_Msk;

	task_run_first();
}

