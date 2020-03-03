#include "stm32f10x.h"
#include "systick.h"
#include "gpio.h"
#include "table.h"
#include "mvc.h"

/****************************************************
��������GPIOInit(void)
�βΣ���
����ֵ����
�������ܣ���ʼ��ϵͳ��GPIO �����Թ����Լ������ʼ����
****************************************************/
void GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//��GPIOA GPIOB GPIOE�ڵ�ʱ�Ӵ�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);
	//��IO������Ϊ���ģʽ
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;
//	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	/*ϵͳ���ӵ�*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*CP �ĵ�ƽ PWM�л���· PE3*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	//GPIO_SetBits(GPIOE,GPIO_Pin_3);
	
	/*��ɫָʾ�� PE2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	/*��ɫָʾ�� PA0*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*��ɫָʾ�� PA1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*��ͣ���ؼ�� PE6*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	/*�̵������� PE4Ϊ��*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOE,GPIO_Pin_4);
	
	/*�̵������� PE3Ϊ��*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOE,GPIO_Pin_3);
}

/****************************************************
��������LedController(void)
�βΣ���
����ֵ����
�������ܣ�
****************************************************/
#define STATE_WAIT					0
#define STATE_UP						1
#define STATE_DOWN					2

static uint8_t ledAppState = STATE_WAIT;    // State tracking variable

void LedTask()
{
		switch(ledAppState)
		{
			case STATE_WAIT:
				vLedTimerSetValue(0);
				ledAppState = STATE_UP;
				break;
			case STATE_UP:
				if(vLedTimerGetValue()>500)
				{
						//�̵ƴ��� 
						if(GetSuperState() == 1 || GetSuperState() == 4)	//����׼����ʼ���
							GPIO_SetBits(GPIOE,GPIO_Pin_2);
						else
							GPIO_ResetBits(GPIOE,GPIO_Pin_2);
						/*
						if(GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2) == 0)
							GPIO_SetBits(GPIOE,GPIO_Pin_2);
						else
							GPIO_ResetBits(GPIOE,GPIO_Pin_2);*/
						
						//�Ƶƴ���
						if(GetSuperState() == 3)	//����׼����ʼ���
							GPIO_SetBits(GPIOA,GPIO_Pin_0);
						else
							GPIO_ResetBits(GPIOA,GPIO_Pin_0);
						
						ledAppState = STATE_WAIT;
				}
				break;
	//		case STATE_DOWN:
			default:break;
		}
}

/****************************************************
����������ͣ���ؼ�� ������� �ͻָ�״̬������ʼ״̬ PE6 ÿ500msִ��һ��
�βΣ���
����ֵ����
�������ܣ�
****************************************************/
void EmergencyStop()
{
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6) == 0)
		{
				MvcStateMachineOpen(); //��
			
				//��ƴ���
				GPIO_ResetBits(GPIOA,GPIO_Pin_1);
		}
		else
		{
				MvcTableInit();	//����
				tableInit();
			
				//��ƴ���
				GPIO_SetBits(GPIOA,GPIO_Pin_1);

		}
}