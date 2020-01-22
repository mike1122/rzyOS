#ifndef RZYOS_MBOX_H
#define RZYOS_MBOX_H

#include "rzyOS.h"

#define rzyOS_mbox_send_normal 0x00
//���뵽��Ϣ������ǰ��
#define rzyOS_mbox_send_front 0x01

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

//��Ϣ�ȴ�����������ģʽ��
uint32_t rzyOS_mbox_wait(rzyOS_mbox_s *rzyOS_mbox, void **msg, uint32_t wait_tick);

//��Ϣ�ȴ�������������ģʽ��
uint32_t rzyOS_mbox_nowait(rzyOS_mbox_s *rzyOS_mbox, void **msg);

//postһ����Ϣ
uint32_t rzyOS_mbox_post(rzyOS_mbox_s *rzyOS_mbox, void *msg, uint32_t notify_option);


#endif