#include "led.h" 



//��ʼ��PC13.��ʹ��GPIOC��ʱ��
//LED IO��ʼ��
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	//ʹ��GPIOCʱ��

	//PC13��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	//LED��ӦIO��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIO
	// GPIO_ResetBits(GPIOC, GPIO_Pin_13);	//����0������
	GPIO_SetBits(GPIOC, GPIO_Pin_13);	//����1������
}
