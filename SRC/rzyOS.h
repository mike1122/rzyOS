#ifndef __RZYOS_H
#define __RZYOS_H

#include <stdint.h>

typedef uint32_t tTaskStack;

typedef struct tTask
{
	tTaskStack *stack;
} tTask; 

extern tTask *currentTask;
extern tTask *nextTask;

void tTaskRunFirst(void);
void tTaskSwitch(void);
#endif
