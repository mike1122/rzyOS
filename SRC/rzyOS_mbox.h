#ifndef RZYOS_MBOX_H
#define RZYOS_MBOX_H

#include "rzyOS.h"

//����ģʽ�� ���뵽�������ĺ��
#define rzyOS_mbox_send_normal 0x00
//���뵽��Ϣ������ǰ��
#define rzyOS_mbox_send_front 0x01

//������Ϣ�ṹ
typedef struct rzyOS_mbox_info_s
{
	//��ǰ��Ϣ����
	uint32_t count;
	//���֧�ֵ���Ϣ����
	uint32_t max_count;
	//�ȴ�����������
	uint32_t task_count;
} rzyOS_mbox_info_s;

//����ṹ
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

//��������
void rzyOS_mbox_flush(rzyOS_mbox_s *rzyOS_mbox);

//�����ɾ��
uint32_t rzyOS_mbox_destory(rzyOS_mbox_s *rzyOS_mbox)

//����״̬��ѯ
void rzyOS_mbox_get_info(rzyOS_mbox_s *rzyOS_mbox, rzyOS_mbox_info_s rzyOS_mbox_info);


#endif
