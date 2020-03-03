/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MVC_H
#define __MVC_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	
#include "adc.h"
#include "usart.h"
#include "can.h"
#include "message.h"
#include "lock.h"
#include "rtc.h"
#include "rc522.h"
#include "modbus.h"
#include "table.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/****************************************************
��������TRANSATION ����ṹ�� 
����������ṹ��������һ�γ����Ϊ mvc״̬��������model����
			����startTime��¼��ʽ�����̿�ʼ�ľ���ʱ�䣬��RTC
			��ã�endTime��¼���Ľ���ʱ�䡣�������¼���״̬����
			���ж�����,�ӽ�����״̬��S6 S7 S8 S9����ʱ���¼��
			transactionStatus��¼��ǰ����µ�״̬ ����--ִ��--����
			�����ߺͽ�������Ҫ��¼���� ֻҪ���˽������
			�ۼ�ʱ����Ҫ��������״̬�ۻ������ۻ��������ۻ����ʵ��
			����һ���£���ȡ���ڵ���ֵ
			���ģʽ��״̬�����£������û�ѡ����и���
			���ʱ�䡢������Ҳ�����û�����
			��׼����ֵ �϶��ɵ�����
			
			�˽ṹ����Ϊm���ⲿ����ͨ��״̬��ת��Ϊm�е����ݣ���ʾ
			ֻ����ʾ����ṹ���е����ݡ�
****************************************************/
typedef struct 
{
		uint8_t transactionStatus;			/*����״̬ ����0 ���1 �ɷ�2 ����3 ��״̬���߼��ı�*/
		uint8_t transactionSponsor[4];	/*������*/
		uint8_t transactionPayer[4];		/*������*/
	
		T_STRUCT rtcTime;				/*��ǰʱ��*/
		T_STRUCT startTime;			/*����ʼʱ�� ������״̬Ϊ1 �� startTimeΪ0*/
		T_STRUCT endTime;				/*�������ʱ�� ������״̬Ϊ0 �� endTimeΪ0*/
	
		uint8_t  chargeMode;						/*���ģʽ ״̬�߼����� 0���� 1ʱ�� 2��� 3�Զ�*/
		uint32_t chargeTime;						/*���ʱ�� ״̬�߼�����*/
		uint32_t chargeMoney;						/*����� ״̬�߼�����*/
		uint32_t chargeElectricity;			/*������ ״̬�߼�����*/
	
		uint32_t accumulateTime;				/*�ۻ�ʱ�� ״̬Ϊ��� ++*/
		uint32_t accumulateMoney;				/*�ۻ����*/
		uint32_t accumulateElectricity;	/*�ۻ����� ״̬Ϊ��� ��ȥ��׼*/
	
		uint8_t  matchStatus;						/*ʱ�� ��� ���� �Ƿ�ﵽ 0��δ�ﵽ 1���ﵽ*/
		uint32_t electricEnergy;				/*��׼����ֵ ״̬Ϊ��� Ϊ0*/
		uint8_t rate;										/*��ǰ����*/
	
		uint8_t moneyOK;								/*������*/
		
		/*��ǰ��ѹ ���� ����*/
		uint16_t voltage;
		uint8_t  current;
		uint32_t w;
		
		/*ǹ����״̬*/
		uint8_t CCstate;
		/*��״̬*/
		CARD_STATUS cardStatus;
		
		/*������ֵ ��������ϵͳ*/
		uint8_t keyValue;
		
		/*��״̬*/
		uint8_t lockStatus;
		
		/*���״̬��״̬*/
		uint8_t chargeStatus;
		
}TRANSACTION;
/* Exported functions ------------------------------------------------------- */
void MvcTableInit(void);
void MvcStateMachineOpen(void);
void MvcStateMachineClose(void);
void MvcStateTimeUpdate(void);
void MvcConditionUpdate(void);
void MvcStateTransitionUpdate(void);
void transactionTableUpdate(void);

void MvcActionExecutor(struct message actionMessage);
uint8_t MvcGetActiveState(void);
uint8_t MvcGetSuperState(void);
TRANSACTION MvcGetTransation();
void MvcSetChargeValue(uint32_t value);
void MvcSetChargeMode(uint8_t mode);
#endif
