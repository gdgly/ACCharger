/**
  ******************************************************************************
  * @file    sheet.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * ���ļ�����������CANCOM_100E�ĸ�ʽת��ģʽ���÷���Ҳ���Ƿ��͹涨��ʽ��485����
  *	֡���ͽ��չ涨��ʽ��485����֡���Լ�����֮ǰ���ͨ���ṹ�幹��֡���ͽ���֮��
	*	��ν���֡
	* ����������ʱ����� ��Ҫ�� ֡�ṹ�Ķ����һ��ʵʱ��������
	* ���ļ����õ�Ӳ���ӿ���Usart3 ��Ĳ�Ҫ��ʹ����
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "sheet.h"
#include "string.h"
#include "usart.h"
#include "CANCOM-100E.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define MODULE1ADDR 0x01		/*ģ���ַ*/
#define MODULE2ADDR 0x02
#define MODULE3ADDR 0x03
#define MODULE4ADDR 0x04
#define MODULE5ADDR 0x05
#define MODULE6ADDR 0x06
#define MODULE7ADDR 0x07
#define MODULE8ADDR 0x08
#define MODULE9ADDR 0x09
#define MODULE10ADDR 0x0a
#define MODULE11ADDR 0x0b
#define MODULE12ADDR 0x0c
#define MODULE13ADDR 0x0d
#define MODULE14ADDR 0x0e
#define MODULE15ADDR 0x0f

#define MODULEON			0x55
#define	MODULEOFF			0xaa
/*ȱʡֵ��������Ч��Χ�ڵĵ���ֵ ���ݴ����Ч�ֶ�*/
#define VOLTAGE_DEFAULT_VALUE 400000	/*ȱʡֵ ��λ��mV*/
#define CURRENT_DEFAULT_VALUE 6000		/*ȱʡֵ ��λ��mA*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint8_t	onOffUpdatePointer = 0;			/*��������ָʾ��ǰҪ���µ���*/
uint8_t	VAUpdatePointer = 0;
uint8_t	statusUpdatePointer = 0;

ModuleSetting moduleSettingTable[MODULENUM];	/*15��ģ��ı�� �ṩ����API*/


/*15��ģ��ĵ�ַ ע������*/
const uint8_t moduleAddr[MODULENUM] = {MODULE1ADDR,MODULE2ADDR,MODULE3ADDR,MODULE4ADDR,
																 MODULE5ADDR,MODULE6ADDR,MODULE7ADDR,MODULE8ADDR,
																 MODULE9ADDR,MODULE10ADDR,MODULE11ADDR,MODULE12ADDR,
																 MODULE13ADDR,MODULE14ADDR,MODULE15ADDR};

/* Private function prototypes -----------------------------------------------*/
void ModuleSettingMonitor(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  ModuleSettingTableUpdate()ͬʱ������������ѹ���������£����²�����
						��ÿ��500msʱ��ڵ��ϣ����µĶ����ɱ���Լ�ά����˳�򣩣��Զ�ִ��
						��������һֱ�ڸ�����
  * @param  
  * @retval 
  */
void ModuleSettingTableUpdate(struct message data)
{
		/*�ж�pointer�Ϸ���*/
		
		/*ִ��CANCOM�ķ��Ͳ��� �����ɱ���ṩ*/
		switch(data.cmd)
		{
			case 0:/*����ģ����*/
				MegmeetTx(VAUpdatePointer,0,moduleSettingTable[VAUpdatePointer]);
				//������ϢmoduleSettingTable[onOffUpdatePointer].mAcurrent;
				//������ϢmoduleSettingTable[onOffUpdatePointer].mVvoltage;
				VAUpdatePointer++;
				if(VAUpdatePointer == MODULENUM)	/*==15 ������Χ��*/
						VAUpdatePointer = 0;
				break;
			case 1:/*����״̬*/
				MegmeetTx(statusUpdatePointer,1,moduleSettingTable[statusUpdatePointer]);
				statusUpdatePointer++;
				if(VAUpdatePointer == MODULENUM)	/*==15 ������Χ��*/
						VAUpdatePointer = 0;
				break;
			case 2:/*���¿�����*/
				//������ϢmoduleSettingTable[onOffUpdatePointer].on
				MegmeetTx(onOffUpdatePointer,2,moduleSettingTable[onOffUpdatePointer]);
				onOffUpdatePointer++;
				if(onOffUpdatePointer == MODULENUM)	/*==15 ������Χ��*/
						onOffUpdatePointer = 0;
				break;
			default:break;
		}
}	

/**
  * @brief  ModuleWriteVA()д���ѹ����
  * @param  
  * @retval 
  */
void ModuleWriteVA(uint8_t moduleNum,\
									 uint32_t voltage,\
									 uint16_t current)
{
		if(moduleNum > MODULENUM)
			return;
		moduleSettingTable[moduleNum].mAcurrent = current;
		moduleSettingTable[moduleNum].mVvoltage = voltage;
}	
/**
  * @brief  ModuleWriteONOFF()д�뿪����
  * @param  
  * @retval 
  */
void ModuleWriteONOFF(uint8_t moduleNum,\
											uint8_t onOff)
{
		if(moduleNum > MODULENUM)
			return;
		moduleSettingTable[moduleNum].on = onOff;
}	
/**
  * @brief  ModuleWriteStatus()д��״̬��
  * @param  
  * @retval 
  */
void ModuleWriteStatus(uint8_t moduleNum,\
											 uint8_t status)
{
		if(moduleNum > MODULENUM)
			return;
		moduleSettingTable[moduleNum].status = status;
}	
/**
  * @brief  void MegmeetInit()
  * @param  
  * @retval 
  */
void ModuleSettingTableInit()
{
		uint8_t i;
		for(i = 0; i< MODULENUM; i++)
		{
				moduleSettingTable[i].on = MODULEON;					/*0x55 ����  0xaa�� ��*/
				moduleSettingTable[i].mVvoltage = VOLTAGE_DEFAULT_VALUE;
				moduleSettingTable[i].mAcurrent = CURRENT_DEFAULT_VALUE;
		}
}	

/**
  * @brief  GetModuleSetting���ڷ��ر�� ���ⲿ���� �ǱȽϹ淶�ķ�ʽ
  * @param  None
  * @retval None
  */	
ModuleSetting * GetModuleSetting()
{
	return moduleSettingTable;
}
uint8_t ModuleAddr(uint8_t moduleNum)
{
	return moduleAddr[moduleNum];
}

/**
  * @brief  ModuleSettingMonitor������ȷ�Լ���� ���ȣ������������һ��Ҫ��ȷ
		֮�󣬱���е��������������ԣ�һ��������ֵ���������������ݻ���ʲô����һ����
		ʵ��ֵ���������������ʵ��״̬��������ֵ����ʵֵ��һ�����ڡ���ô����ֵ�ڱ仯
��		���ڹ̶�t�����ڣ����û�дﵽ����ֵ������ֵ�����ڷ�Χ�ڣ���ô������Ϊģ��
		�Ѿ�ʧȥ�������������������ˣ�������ﵽ������ֵ����ô˵����ȷ�������������
		ֵ�ڲ�ͣ�ر仯����ô��ʵֵ��ȻҲ���ڲ�ͣ�仯����ô��ʵֵ���жϾ�û�������ˣ�
		��ô��Ҫ��С��ʵֵ���ж�Ƶ�ʣ�ʹ����������Ĳο���ֵ��
  * @param  None
  * @retval None
  */	
void ModuleSettingMonitor()
{
		uint8_t counter;
		for(counter = 0; counter < MODULENUM; counter++)
		{
				if(moduleSettingTable[counter].on == MODULEOFF)
				{
						moduleSettingTable[counter].mVvoltage = VOLTAGE_DEFAULT_VALUE;
						moduleSettingTable[counter].mAcurrent = CURRENT_DEFAULT_VALUE;
				}
				else
				{
						if(moduleSettingTable[counter].mVvoltage < 400000)
								moduleSettingTable[counter].mVvoltage = VOLTAGE_DEFAULT_VALUE;
						if(moduleSettingTable[counter].mVvoltage > 600000)
								moduleSettingTable[counter].mVvoltage = CURRENT_DEFAULT_VALUE;
						if(moduleSettingTable[counter].mAcurrent > 6000)
								moduleSettingTable[counter].mVvoltage = 6000;
				}
		}
}
