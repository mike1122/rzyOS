#include "rzyOS_schedule.h"
#include "rzyOSarch.h"
#include "rzyOS_wqueue.h"



//��ǰ����ָ��
task_tcb_s *currentTask;
//��һ��׼��ִ�е�����ָ��
task_tcb_s *nextTask;
//��������ָ��
task_tcb_s *idleTask;

//����һ�������ȼ����ֵ�λͼ�ṹ
bitmap_s bitmap_taskprio;

//���� RZYOS_PRIO_COUNT��32����˫���������ṹ�� ����ͬ���ȼ��ľ����������
list_t task_ready_table[RZYOS_PRIO_COUNT];

//����������
uint8_t schedLockCount;

//������ʱ����
list_t task_delay_list;


//ϵͳ���ļ���
uint32_t tick_count;

#if RZYOS_ENABLE_CPU_DETECT == 1
//�����������н���ͳ��
uint32_t idle_count;
//�����������������н���ͳ��
uint32_t idle_max_count;
//cpuʹ���ʼ�⺯��
static void check_cpu_usage_detect(void);
#endif


//��λͼ���ҵ�������������ȼ�
//���վ�����������ȼ��ھ��������б��л�ȡ��һ������ڵ�
//�ӵ�һ������ڵ�õ���Ӧ��task��TCB
task_tcb_s *task_highest_ready(void)
{
	//���ҵ�λͼ�б�ǵ�������ȼ���
	uint32_t highest_prio = bitmap_get_first_set(&bitmap_taskprio);
	//�ҵ������ȼ����е�һ����������ڵ�
	node_t *node = list_first_node(&task_ready_table[highest_prio]);
	//��������ָ��
	return node_parent(node, task_tcb_s, link_node);
}

//�������ȼ�,�Ѿ��������������ڵ�����������list��
void task_insert_ready_list(task_tcb_s *task_tcb)
{
	//������ڵ㰴��˳����ӵ����ȼ�������
	list_add_last(&task_ready_table[task_tcb -> prio], &(task_tcb -> link_node));
	bitmap_set(&bitmap_taskprio, task_tcb -> prio);
}

//������Ӿ���������ɾ�Ƴ�(�ڲ�ʹ��)
void task_remove_ready_list(task_tcb_s *task_tcb)
{
	//��taskʹ����ʱ����,����ô˺���

	//��Ϊ���������µ���ʱ, ����Ҫ�ھ���list��ɾ����ǰ���������ڵ�
	list_remove_pos_node(&task_ready_table[task_tcb -> prio], &(task_tcb -> link_node));
	
	if (0 == list_count(&task_ready_table[task_tcb -> prio]))
	{
		bitmap_clean(&bitmap_taskprio, task_tcb -> prio);
	}
}

//������Ӿ����������Ƴ�
void rzyOS_task_ready_list_remove(task_tcb_s *task_tcb)
{
	list_remove_pos_node(&task_ready_table[task_tcb -> prio], &(task_tcb -> link_node));
	
	if (0 == list_count(&task_ready_table[task_tcb -> prio]))
	{
		bitmap_clean(&bitmap_taskprio, task_tcb -> prio);
	}
}

//task����, �л���������ȼ���,��ʱ�����task
//����������״̬ʱ��Ȼ�ھ����б���
void task_schedule(void)
{
	task_tcb_s *tempTask;
	uint32_t status = task_enter_critical();
	
	//���������� > 0 �� ��ֹ����
	if (schedLockCount > 0)
	{
		task_exit_critical(status);
		return ;
	}
	
	tempTask = task_highest_ready();
	
	if (tempTask != currentTask)
	{
		nextTask = tempTask;
		task_switch(); //����pendSV
	}
	
	task_exit_critical(status);
}

//��ʼ��λͼ
//��ʼ�����������list����(���鰴�����ȼ�����)
void task_schedule_init(void)
{
	int i;
	
	schedLockCount = 0;
	bitmap_init(&bitmap_taskprio);
	for (i = 0; i < RZYOS_PRIO_COUNT; i ++)
	{
		list_init(&task_ready_table[i]);
	}
}

//����ʧ�� , ���������� + 1
void task_schedule_disable(void)
{
	uint32_t status = task_enter_critical();
	
	if (schedLockCount < 0xff)
	{
		schedLockCount ++;
	}
	
	task_exit_critical(status);
}

//����ʹ�� , ���������� - 1
void task_schedule_enable(void)
{
	uint32_t status = task_enter_critical();
	
	if (schedLockCount > 0)
	{
		schedLockCount --;

		//�����ü���0 , ���������
		if (0 == schedLockCount)
		{
			task_schedule();
		}
	}
	
	task_exit_critical(status);
}

//��ʼ����ʱ�б�list
void task_delay_list_init(void)
{
	list_init(&task_delay_list);
}



//����ʱlist�м��뵱ǰ�������ʱ�ڵ�
void delay_list_insert_time_node(task_tcb_s *task_tcb, uint32_t ticks)
{
	//��taskʹ����ʱ����,����ô˺���

	//�ڵ�ǰ�����TCB��д����Ҫ����ʱ
	task_tcb -> delayTicks = ticks;
	list_add_first(&task_delay_list, &(task_tcb -> delay_node));
	//��ע����״̬Ϊ��ʱ�ȴ�
	task_tcb -> task_status |= RZYOS_TASK_STATUS_DELAY;
}

//����������ʱ������ɾ��(�ڲ�����)
void delay_list_remove_time_node(task_tcb_s *task_tcb)
{
	list_remove_pos_node(&task_delay_list, &(task_tcb -> delay_node));
	//���������ʱ�ȴ�״̬
	task_tcb -> task_status &= ~RZYOS_TASK_STATUS_DELAY;
}

//����������ʱ������ɾ��(�ⲿ����)
void rzyOS_task_delay_list_remove(task_tcb_s *task_tcb)
{
	delay_list_remove_time_node(task_tcb);
}

//ϵͳ���ĳ�ʼ��
void rzyOS_tick_count_init(void)
{
	tick_count = 0;
}

//systick�жϵ��ô˺���
void task_systemtick_handler(void)
{
	node_t *node;
	
	uint32_t status = task_enter_critical();
	
	//������ʱ�ȴ�����
	for (node = task_delay_list.head_node.next_node; node != &(task_delay_list.head_node); node = node -> next_node)
	{
		//������ʱ�ڵ㣬��ȡ������ƿ�
		task_tcb_s *task_tcb =  (task_tcb_s *)node_parent(node, task_tcb_s, delay_node);
		//������ʱ�ݼ�
		task_tcb -> delayTicks --;

		//��ʱ��Ϊ0
		if (0 == task_tcb -> delayTicks)
		{
			//�������Դ����¼��ȴ�״̬
			if (task_tcb -> wait_event)
			{
				//����¼���δ����,����ʱ����, ����¼��ȴ��б�ɾ��
				rzyOS_event_remove(task_tcb, (void *)0, error_timeout);
			}

			delay_list_remove_time_node(task_tcb);
			
			task_insert_ready_list(task_tcb);
		}
	}
	
	//ʱ��Ƭ��ת
	//��Ϊ���ȵ�nextTask�������ȼ��б�ĵص�һ���ڵ�
	//���Ի���ʱ��Ƭ����Ҫ�Ե�һ���ڵ����,Ҳ����currentTask
	currentTask -> slice --;
	//�жϵ�ǰ����ʱ��Ƭ�Ƿ�����
	if (0 == currentTask -> slice)
	{
		//�������, �ҵ�ǰ�����ͬ���ȼ�����list��������, ���л���ת
		if (list_count(&task_ready_table[currentTask -> prio]) > 0)
		{
			//�Ƴ�ͬ���ȼ�����list�е�����ڵ�, �����뵽���һ���ڵ�, ��ɻ���ͬ���ȼ���ʱ��Ƭ����
			remove_list_first(&task_ready_table[currentTask -> prio]);
			list_add_last(&task_ready_table[currentTask -> prio], &(currentTask -> link_node));
			
			//����ʱ��Ƭ
			currentTask -> slice = RZYOS_SLICE_MAX;
		}
	}

	//ϵͳ�����ۼ�
	tick_count ++;

#if RZYOS_ENABLE_CPU_DETECT == 1

	//cpuʹ���ʼ��
	check_cpu_usage_detect();
#endif
	
	task_exit_critical(status);

#if RZYOS_ENABLE_WQUEUE == 1
	//ϵͳtick�� �����Ե��ù������д�����
	rzyOS_wqueue_tick_handle();
#endif
	
	task_schedule();
}

//��������������ȵ���ʱ
void delay(int count)
{
	while(-- count > 0);
}


#if RZYOS_ENABLE_CPU_DETECT == 1

//cpuʹ����
static float cpu_usage;
//cpuͬ���ȴ���־
static uint32_t cpu_sync_flag;

//cpuʹ���ʼ��ģ�������ʼ��
static void cpu_usage_state_init(void)
{
	idle_count = 0;
	idle_max_count = 0;
	cpu_usage = 0.0f;
	cpu_sync_flag = 0;
}

//cpuʹ���ʼ�⺯��
static void check_cpu_usage_detect(void)
{
	//����˺��������cpuͬ��
	//��Ϊcpuδ����ͬ��״̬������λcpu_sync_flag
	//ֻ��λcpu_sync_flagһ��
	if (0 == cpu_sync_flag)
	{
		//��λcpu_sync_flag
		cpu_sync_flag = 1;
		return ;
	}

	//ϵͳ���ļ�����1��
	if (ONE_SECOND == tick_count)
	{
		//�ѵ�һ��Ŀ������������ɽ��ļ�����ֵ
		idle_max_count = idle_count;
		idle_count = 0;

		//��������
		task_schedule_enable();
	}
	//ÿ��tick_count����1��
	else if (0 == tick_count % ONE_SECOND)
	{
		//����cpuʹ����
		cpu_usage = 100 - (float)100.0 * (float)idle_count / (float)idle_max_count;
		idle_count = 0;
	}
}

static void cpu_tick_sync(void)
{
	//cpuδͬ������while(1)�ȴ�
	while (0 == cpu_sync_flag)
	{
		;;;;;
	}
}

//��ȡcpuʹ���ʣ�������
float rzyOS_get_cpu_usage(void)
{
	float usage = 0.0f;

	uint32_t status = task_enter_critical();
	usage = cpu_usage;
	task_exit_critical(status);

	return usage;
}

#endif

static task_tcb_s tcb_task_idle;
static tTaskStack idleTaskEnv[RZYOS_IDLETASK_STACK_SIZE];

void idle_task_entry(void *param)
{
	//�ر��������
	task_schedule_disable();

//	//app�����ʼ��
//	rzyOS_app_init();

#if RZYOS_ENABLE_WQUEUE == 1
	//�������������ʼ��
	rzyOS_wqueue_task_init();
#endif

//	//�趨systick�ж�ʱ������
//	 set_systick_period(RZYOS_TICK_MS);

#if RZYOS_ENABLE_CPU_DETECT == 1
	//cpuͬ���ȴ�
	cpu_tick_sync();
#endif 

	for (;;)
	{
#if RZYOS_ENABLE_CPU_DETECT == 1
		uint32_t status = task_enter_critical();
		//�����������н��ļ���
		idle_count ++;
		task_exit_critical(status);
#endif 
	}
}

void rzyOS_kernel_init(void)
{
	task_schedule_init();
	
	task_delay_list_init();

#if RZYOS_ENABLE_WQUEUE == 1
	rzyOS_wqueue_module_init();
#endif

	//ϵͳ���ı�����ʼ��
	rzyOS_tick_count_init();

#if RZYOS_ENABLE_CPU_DETECT == 1
	//cpu ״̬���ģ�������ʼ��
	cpu_usage_state_init();
#endif

#if (RZYOS_ENABLE_MEMORY == 1) && (RZYOS_MM1_USE == 1)
	//��ʼ��memory��ʽ1�ڴ��
	rzyOS_memory_mm1_init();
#endif

	task_init(&tcb_task_idle, idle_task_entry, (void *)0, RZYOS_IDLETASK_PRIO, idleTaskEnv, RZYOS_IDLETASK_STACK_SIZE);
	idleTask = &tcb_task_idle;
}
