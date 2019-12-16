#ifndef __RZYOS_SEMAPHORE_H
#define __RZYOS_SEMAPHORE_H

#include <stdint.h>
#include "rzyOS_event.h"

//�������������ź�ֵ
#define NOLIMITE_MAX_COUNT 0

typedef struct rzyOS_sem_s
{
	rzyOS_ecb_s rzyOS_ecb;
	//����ֵ
	uint32_t count;
	//���֧�ּ���ֵ
	uint32_t max_count;
} rzyOS_sem_s;

//�źų�ʼ��
void rzyOS_sem_init(rzyOS_sem_s *sem, uint32_t start_count, uint32_t max_count);

#endif
