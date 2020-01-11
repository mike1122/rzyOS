#ifndef RZYOS_TASK_H
#define RZYOS_TASK_H

#include "rzyOS_event.h"

//(����״̬)�������
#define RZYOS_TASK_STATUS_READY 0
//(����״̬)����ȴ�
#define RZYOS_TASK_STATUS_DELAY (1 << 1)
//(����״̬)�������
#define RZYOS_TASK_STATUS_SUSPEND (1 << 2)
//(����״̬)����ɾ��
#define RZYOS_TASK_STATUS_DELETE (1 << 3)


//(����״̬)��16λ��Ϊ�¼��ȴ���־
#define RZYOS_TASK_WAIT_MASK (0xff << 16)


//���������ջ��ַΪ32λ����
typedef uint32_t tTaskStack;


//��������
typedef enum rzyOS_error_e
{
	//�޴���
	error_no_error = 0,
	//��ʱ�ȴ�
	error_timeout = 1,
	//�¼�(�ź���&����)����Դ
	error_resource_unvaliable = 2,
	//�ź����Ѿ���ɾ��
	error_sem_delete = 3,
	//��Դ����
	error_resource_full = 4,
} rzyOS_error_e;


//task.h��event.h�ṹ�廥�����, ����һ�� ԭ������ ����
typedef struct rzyOS_ecb_s rzyOS_ecb_s;


//task TCB
typedef struct task_tcb_s
{
	//�����ջָ��
	tTaskStack *stack;

	//����ڵ�
	node_t link_node;

	//����ע����ʱ����
	uint32_t delayTicks;

	//������ʱ�ڵ�
	node_t delay_node;

	//�������ȼ�
	uint32_t prio;

	//����״̬
	uint32_t task_status;

	//ʱ��Ƭ
	uint32_t slice;
	
	//������������
	uint32_t suspend_count;
	
	//����ɾ��
	//�������callback����
	void (*clean)(void *param);
	//�������callback��������
	void *clean_param;
	//��������ɾ��״̬���
	uint8_t request_delete_flag;
	
	//�¼����ƿ�
	rzyOS_ecb_s *wait_event;
	//�ȴ��¼�����Ϣ����
	void *event_msg;
	//�ȴ��¼��Ľ��
	uint32_t wait_event_result;
	//������¼�����
	uint32_t wait_flag_type;
	//������¼���־
	uint32_t event_flag;
} task_tcb_s;


//task info struct
typedef struct rzyOS_task_info_s
{
	//����ע����ʱ����
	uint32_t delay_ticks;
	//�������ȼ�
	uint32_t prio;
	//����״̬
	uint32_t task_status;
	//ʱ��Ƭ(���ͬ���ȼ�����)
	uint32_t slice;
	//������������
	uint32_t suspend_count;
} rzyOS_task_info_s;


//task��ʼ��
void task_init(task_tcb_s *task, void (*entry)(void *), void *param, uint32_t prio, tTaskStack *stack);

//����Ĺ���
void rzyOS_task_suspend(task_tcb_s *task);

//����Ļ���
void rzyOS_task_wakeup(task_tcb_s *task);

//��������ص�����
void rzyOS_task_clean_callback(task_tcb_s *task, void (*clean)(void *param), void *param);

//����ǿ��ɾ������
void rzyOS_task_force_delete(task_tcb_s *task);

//��������ɾ������
void rzyOS_task_request_delete(task_tcb_s *task);

//������������ɾ���ļ�⺯��
uint8_t rzyOS_task_request_delete_check(void);

//��������ɾ������
void rzyOS_task_delete_self(void);

//��ȡ������Ϣ
void rzyOS_task_get_info(task_tcb_s *task, rzyOS_task_info_s *info);

#endif
