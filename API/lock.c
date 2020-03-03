/**
  ******************************************************************************
  * @file    lock.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *	�˺�����������װ�ã�����װ��ֻ������״̬ ���͹أ�ͨ����ת�ͷ�תʵ��
		���ڵ������ת��Ҫһ��ʱ�䣬��˲��������ڲ��������еȴ�ִ�У���֧��
		�жϲ���
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "lock.h"
#include "usart.h"
#include "string.h"
#include "systick.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define CMD_QUEUE_SIZE  10
/* Private variables ---------------------------------------------------------*/
uint8_t lockStatus; 			/*����װ��Ŀǰ��״̬ 0���� 1���ر�*/
struct 
{
	uint8_t cmdQueue[CMD_QUEUE_SIZE];
	uint8_t front;
	uint8_t rear;
	uint8_t size;
}lockCmdQueue;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  �˺���������Ϣ���� �����ǻ᲻�ᱻ�ж�Ӱ��
  * @param  ���� �л���־λ ��Ϣ��ֵ 
  * @retval ��� 0 �ɹ� 1 ��Ϣ�������� 
	* @usage  LockSet(LOCK);  LockSet(UNLOCK);
  */
uint8_t LockSet(uint8_t cmd)	
{
		uint8_t temp;
		if((((lockCmdQueue.rear) + 1)%CMD_QUEUE_SIZE) == lockCmdQueue.front)		//����
		{
				return 1;
		}
		else
		{
				temp = lockCmdQueue.rear;
				lockCmdQueue.cmdQueue[temp] = cmd;
				lockCmdQueue.rear = (temp + 1) % CMD_QUEUE_SIZE;
				return 0;
		}
}
/**
  * @brief  �˺�����ȡ��Ϣ���� �����ǻ᲻�ᱻ�ж�Ӱ��
  * @param  ���� ��Ϣ���е�ָ�� ��Ϣ��ָ�루���ó���ά���� 
  * @retval ��� 0 �ɹ� 1 ��Ϣ�����ѿ�
  */
uint8_t cmdDequeue(uint8_t * data_pointer)
{
		uint8_t temp;
		if(lockCmdQueue.front == lockCmdQueue.rear)							//����
		{
				return 1;
		}
		else
		{
				temp = lockCmdQueue.front;
				*data_pointer = lockCmdQueue.cmdQueue[temp];
				lockCmdQueue.front = (temp + 1)%CMD_QUEUE_SIZE;
				return 0;
		}
}
/**
  * @brief  GPIO_Lock_Configuration �����������
  * @param  
  * @retval 
  */
void GPIO_Lock_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//��GPIOA GPIOB GPIOE�ڵ�ʱ�Ӵ�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);

	/*��ת��ת����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	/*mos������*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	
	/*�������*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

/**
  * @brief LockInit ��ʼ��״̬
  * @param  
  * @retval 
  */
void LockInit(void)
{
		GPIO_Lock_Configuration();
		memset(&lockCmdQueue,0,sizeof(lockCmdQueue));	/*������г�ʼ��*/
		lockStatus = 0;																/*��״̬��ʼ��*/
}

/**
  * @brief GetLockStatus ��õ�ǰ�������صĿ���״̬ 0���� 1����
  * @param  
  * @retval 
  */
uint8_t GetLockStatus(void)
{				
		return lockStatus;
}

/**
  * @brief LockTask��ȡ���ִ�й�������װ�õĶ���
  * @param  
  * @retval 
  */
#define STATE_WAIT					0
#define STATE_READ_CMD			1
#define STATE_CMD_EXE				2
#define STATE_READ_STATUS		3
#define STATE_CMDX_EXE			4


static uint8_t lockAppState = STATE_WAIT;    // State tracking variable

void LockTask(void)
{
		static uint8_t cmd;

    switch(lockAppState)
    {
				case STATE_WAIT:
						lockAppState = STATE_READ_CMD;
						break;
				case STATE_READ_CMD:
						if(cmdDequeue(&cmd) == 0)			/*������*/
						{
								/*��ʱ״̬��������*/
								if(lockStatus != cmd)
								{
										lockAppState = STATE_CMD_EXE;
								}
						}
						break;
				case STATE_CMD_EXE:
						if(cmd == 0)
						{
								GPIO_SetBits(GPIOB,GPIO_Pin_8);
								GPIO_SetBits(GPIOB,GPIO_Pin_9);
						}
						else
						{
								GPIO_ResetBits(GPIOB,GPIO_Pin_8);
								GPIO_SetBits(GPIOB,GPIO_Pin_9);
						}
						lockAppState = STATE_READ_STATUS;
						break;
				case STATE_READ_STATUS:
						/*��������ٽ����*/
						if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) == cmd)
						{
								vLockTimerSetValue(0);				/*��ʼ��ʱ*/
								lockAppState = STATE_CMDX_EXE;
						}
						break;
				case STATE_CMDX_EXE:
						if(cmd == 0)
						{
								if(vLockTimerGetValue() > 500)
								{
										GPIO_ResetBits(GPIOB,GPIO_Pin_9);
										lockStatus = cmd;
										lockAppState = STATE_WAIT;
								}
						}
						else
						{
								if(vLockTimerGetValue() > 30)
								{
										GPIO_ResetBits(GPIOB,GPIO_Pin_9);
										lockStatus = cmd;
										lockAppState = STATE_WAIT;
								}
						}
						break;
				default:break;
		}
}
