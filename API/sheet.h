/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SHEET_H
#define __SHEET_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
/* Exported macro ------------------------------------------------------------*/
#define MODULENUM   15				/*ģ�����*/

/* Exported types ------------------------------------------------------------*/
typedef struct						/*megmeetÿһ��ģ��Ľṹ�嶨��*/
{
	uint8_t on;								/*�趨ֵ*/
	uint16_t mAcurrent;
	uint32_t mVvoltage;
	
	uint16_t statusCurrent;		/*ע����X10 A*/
	uint16_t statusVoltage;		/*ע����X10 V*/
	uint16_t status;					/*״ֵ̬*/
	
	uint8_t textChanged;			/*��עֵ�Ƿ�ı䣬��ÿһ�������ݸı��ʱ����һ*/
														/*�ڶ���ֵ����֮������*/
	uint8_t duration;					/*��λ��s ÿһ��textChanged�����㣬���ҿ�ʼ��ʱ*/
}ModuleSetting;

void ModuleSettingTableUpdate(struct message data);
void ModuleWriteVA(uint8_t moduleNum,\
									 uint32_t voltage,\
									 uint16_t current);
void ModuleWriteONOFF(uint8_t moduleNum,\
											uint8_t onOff);
void ModuleWriteStatus(uint8_t moduleNum,\
											 uint8_t status);
void ModuleSettingTableInit(void);
ModuleSetting * GetModuleSetting(void);
uint8_t ModuleAddr(uint8_t moduleNum);
#endif
