#include "rzyOS_schedule.h"
#include "rzyOSarch.h"


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

    STMDB   R0!, {R4-R11}               // ��R4~R11���浽��ǰ����ջ��Ҳ����PSPָ��Ķ�ջ
    VSTMDB  R0!, {S16-S31}               // ���渡��S16-31
    BL      saveAndLoadStackAddr        // ���ú���������ͨ��R0���ݣ�����ֵҲͨ��R0���� 
    VLDMIA  R0!, {S16-S31}               // �ָ�����S16-31
    LDMIA   R0!, {R4-R11}               // ����һ����Ķ�ջ�У��ָ�R4~R11

    MSR     PSP, R0
    MOV     LR, #0xFFFFFFED             // ָ�������쳣ʱʹ��PSP��ע�⣬��ʱLR���ǳ��򷵻ص�ַ
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
