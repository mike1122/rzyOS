#ifndef __RZYOS_H
#define __RZYOS_H

#include <stdint.h>

typedef uint32_t tTaskStack;

typedef struct tTask
{
	tTaskStack *stack;
	uint32_t delayTicks;
} tTask; 

extern tTask *currentTask;
extern tTask *nextTask;

uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

void tTaskRunFirst(void);
void tTaskSwitch(void);
#endif
