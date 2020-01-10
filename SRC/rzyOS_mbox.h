#ifndef RZYOS_MBOX_H
#define RZYOS_MBOX_H

#include "rzyOS.h"

typedef struct rzyOS_mbox_s
{
	//�¼����ƿ�
	rzyOS_ecb_s rzyOS_ecb;
	//�������
	uint32_t count;
	//����λ��
	uint32_t read;
	//д��λ��
	uint32_t write;
	//���ռ�
	uint32_t max_count;
	//buffer��ַ
	void **msg_buffer;
} rzyOS_mbox_s;

//�����ʼ��
void rzyOS_mbox_init(rzyOS_mbox_s *rzyOS_mbox, void **msg_buffer, uint32_t max_count);

#endif
