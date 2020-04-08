#ifndef __RZYOS_SEMAPHORE_H
#define __RZYOS_SEMAPHORE_H

#include <stdint.h>
#include "rzyOS_event.h"
#include "rzyOS.h"

//�������������ź�ֵ
#define NOLIMITE_MAX_COUNT 0

//�ź����ṹ��
typedef struct rzyOS_sem_s
{
	//�¼����ƿ�
	rzyOS_ecb_s rzyOS_ecb;
	//����ֵ
	uint32_t count;
	//���֧�ּ���ֵ
	//max_count = 0 : ����ֵ������
	uint32_t max_count;
} rzyOS_sem_s;

//�ź�����Ϣ
typedef struct rzyOS_sem_info_s
{
	//��ǰ�ź�������ֵ
	uint32_t sem_count;
	//��ǰ�ź������֧�ּ���ֵ
	uint32_t max_count;
	//��ǰ�ź����ȴ�����������
	uint32_t task_count;
} rzyOS_sem_info_s;

//�ź�����ʼ������
void rzyOS_sem_init(rzyOS_sem_s *sem, uint32_t start_count, uint32_t max_count);

//�ź��������ȴ�����
uint32_t rzyOS_sem_wait(rzyOS_sem_s *sem, uint32_t wait_ticks);

//�ź����������ȴ�����
uint32_t rzyOS_sem_no_wait(rzyOS_sem_s *sem);

//�ź����ͷź���
void rzyOS_sem_post(rzyOS_sem_s *sem);

//�ź�������
uint32_t rzyOS_sem_destroy(rzyOS_sem_s *sem);

//�ź�����Ϣ��ú���
void rzyOS_sem_get_info(rzyOS_sem_s *sem, rzyOS_sem_info_s *sem_info);


#endif
