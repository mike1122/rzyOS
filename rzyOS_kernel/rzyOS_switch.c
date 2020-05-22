#include "rzyOS_schedule.h"
#include "rzyOSarch.h"


//中断控制器地址
#define NVIC_INT_CTRL	0xe000ed04
//触发PendSV
#define	NVIC_PENDSVSET	0X10000000
//PendSV优先级控制地址
#define	NVIC_SYSPRI2	0xe000ed22
//PendSV设置为最低优先值255
#define	NVIC_PENDSV_PRI	0x000000ff



#define MEM32(addr)	*(volatile unsigned long *)(addr)
#define MEM8(addr)	*(volatile unsigned char *)(addr)

//进入临界区保护
uint32_t task_enter_critical(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	return primask;
}

//退出临界区保护
void task_exit_critical(uint32_t status)
{
	__set_PRIMASK(status);
}


#ifdef ARMCM3_SIM
//pendsv中断服务函数
__asm void PendSV_Handler(void)
{
	IMPORT currentTask					// IMPORT后面跟随函数名或变量名，作用相当于C中的extern关键字，指出这些全局符号是在其它源文件中定义的
	IMPORT nextTask
	
	MRS R0, PSP							// MRS <gp_reg>, <special_reg> ;读特殊功能寄存器的值到通用寄存器
	CBZ R0, PendSVHander_nosave			// 比较，如果结果为 0 就转移
	STMDB R0!, {R4-R11}					// 存储R4-R11到 R0 地址处。每存一个字后(!) Rd 自增一次，32位宽度
	LDR R1, =currentTask				// 取currentTask这个变量符号的地址写到R1！注意，不是取currentTask的值
	LDR R1, [R1]						// R1= *R1
	STR R0, [R1]						// 把R0存储到R1地址处
	
PendSVHander_nosave
	LDR R0, =currentTask
	LDR R1, =nextTask
	LDR R2, [R1]
	STR R2, [R0]
	
	LDR R0, [R2]
	LDMIA R0!, {R4-R11}					// 从 R0 处读取多个字。写入R4-R11中, 每读一个字后 R0 自增一次，16位宽度
	
	MSR PSP, R0							// MRS <special_reg>,  <gp_reg> ;存储gp_reg的值到特殊功能寄存器
	ORR LR, LR, #0X04					// ORR按位或 标记使用PSP
	BX LR								// 最后返回，此时任务就会从堆栈中取出LR值，恢复到上次运行的位
}
#endif


#ifdef STM32F40XX
//pendsv中断服务函数
__asm void PendSV_Handler(void)
{
    IMPORT saveAndLoadStackAddr
    
    // 切换第一个任务时,由于设置了PSP=MSP，所以下面的STMDB保存会将R4~R11
    // 保存到系统启动时默认的MSP堆栈中，而不是某个任务
    MRS     R0, PSP                 

    STMDB   R0!, {R4-R11}               // 将R4~R11保存到当前任务栈，也就是PSP指向的堆栈
    VSTMDB  R0!, {S16-S31}               // 保存浮点S16-31
    BL      saveAndLoadStackAddr        // 调用函数：参数通过R0传递，返回值也通过R0传递 
    VLDMIA  R0!, {S16-S31}               // 恢复浮点S16-31
    LDMIA   R0!, {R4-R11}               // 从下一任务的堆栈中，恢复R4~R11

    MSR     PSP, R0
    MOV     LR, #0xFFFFFFED             // 指明返回异常时使用PSP。注意，这时LR不是程序返回地址
    BX      LR
}

uint32_t saveAndLoadStackAddr(uint32_t stackAddr)
{
	//第一次切换时， 当前任务tcb为0， 所以不会保存
	if (currentTask != (task_tcb_s *)0)
	{
		//保存堆栈地址
		currentTask -> stack = (uint32_t *)stackAddr;
	}

	currentTask = nextTask;

	//取下一任务堆栈地址
	return (uint32_t)currentTask -> stack;
}
#endif


//从MSP转换PSP,配置pendsv优先级
//change MSP to PSP , then setup pendSV priority and trigger pendSV
void task_run_first(void)
{
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

//触发pendsv中断
//trigger pendSV
void task_switch(void)
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}
