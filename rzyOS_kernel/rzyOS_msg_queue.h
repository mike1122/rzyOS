#ifndef RZYOS_MSG_QUEUE_H
#define RZYOS_MSG_QUEUE_H

#include "rzyOS_schedule.h"

//����ģʽ�� ���뵽�������ĺ��
#define rzyOS_msg_queue_send_normal 0x00
//���뵽��Ϣ������ǰ��
#define rzyOS_msg_queue_send_front 0x01

//��Ϣ������Ϣ�ṹ
typedef struct rzyOS_msg_queue_info_s
{
	//��ǰ��Ϣ����
	uint32_t count;
	//���֧�ֵ���Ϣ����
	uint32_t max_count;
	//�ȴ�����������
	uint32_t task_count;
} rzyOS_msg_queue_info_s;

//��Ϣ���нṹ
typedef struct rzyOS_msg_queue_s
{
	//�¼����ƿ�
	rzyOS_ecb_s rzyOS_ecb;
	//��Ϣ���м���
	uint32_t count;
	//����λ��
	uint32_t read;
	//д��λ��
	uint32_t write;
	//���ռ�
	uint32_t max_count;
	//buffer��ַ
	void **msg_buffer;
} rzyOS_msg_queue_s;

//��Ϣ���г�ʼ��
void rzyOS_msg_queue_init(rzyOS_msg_queue_s *rzyOS_msg_queue, void **msg_buffer, uint32_t max_count);

//��Ϣ�ȴ�����������ģʽ��
uint32_t rzyOS_msg_queue_wait(rzyOS_msg_queue_s *rzyOS_msg_queue, void **msg, uint32_t wait_tick);

//��Ϣ�ȴ�������������ģʽ��
uint32_t rzyOS_msg_queue_nowait(rzyOS_msg_queue_s *rzyOS_msg_queue, void **msg);

//postһ����Ϣ
uint32_t rzyOS_msg_queue_post(rzyOS_msg_queue_s *rzyOS_msg_queue, void *msg, uint32_t notify_option);

//��Ϣ���е����
void rzyOS_msg_queue_flush(rzyOS_msg_queue_s *rzyOS_msg_queue);

//��Ϣ���е�ɾ��
uint32_t rzyOS_msg_queue_destory(rzyOS_msg_queue_s *rzyOS_msg_queue);

//��Ϣ����״̬��ѯ
void rzyOS_msg_queue_get_info(rzyOS_msg_queue_s *rzyOS_msg_queue, rzyOS_msg_queue_info_s *rzyOS_msg_queue_info);


#endif
