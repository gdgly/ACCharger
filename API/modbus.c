/**
  ******************************************************************************
  * @file    modbus.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "modbus.h"
#include "usart.h"
#include "crc8_16.h"
#include "string.h"
#include "CANCOM-100E.h"
#include "iDM100E.h"
#include "AM2301.h"
#include "adc.h"
#include "table.h"
#include "rc522.h"
#include "mvc.h"
#include "rtc.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define SLAVE_ADDRESS 0x01

/* Private macro -------------------------------------------------------------*/
#define STATE_WAIT					0
#define STATE_FRAME_CHECK		1
#define STATE_FRAME_ADDR		2
#define STATE_FRAME_FUNC		3

/* Private variables ---------------------------------------------------------*/
uint8_t rxFrame[40];
uint8_t txFrame[120];

ModuleSetting * moduleTable;							/*������صĹ���ģ����*/
float * piDM100EVoltage;
float * piDM100ECurrent;
float * piDM100EW;
float * pTemperature;
float * pHumidity;

uint8_t keyValue;							/*��ֵ���� ֮���Բ��ö��� ����Ϊû�б�Ҫ 
																��֮��Ӧ��API��GetKeyValue()*/

static uint8_t mAppState = STATE_WAIT;    // State tracking variable

/* Private function prototypes -----------------------------------------------*/
uint8_t crcCheck(uint8_t * pD, uint8_t num);
void modbusFunction(uint8_t * pD, uint8_t num);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ModbusInit()��ʼ�� modbusͨ��ʱ���ݵı�� moduleTable�ȵ�....
  * @param  None
  * @retval None
  */
void ModbusInit()
{
		moduleTable = GetModuleSetting();				/*��ȡ���*/
		piDM100EVoltage = GetVoltage();					/*�������ĵ�ѹ��������ָ��*/
		piDM100ECurrent = GetCurrent();
		piDM100EW = GetW();
		pTemperature = GetTemperature();
		pHumidity = GetHumidity();
}
/**
  * @brief  Main program ����Эͬ������ ������������ 0.�ȴ����� 1.������ 2.�жϵ�ַ 3.
	*	�жϹ��� 4.���ݹ���ɨ������ 5.�������� 6.�ȴ�����
	*					ע��:����Ҫ��static�ģ�
  * @param  None
  * @retval None
  */
void ModbusTask()
{
//		static uint16_t timeCounter;
//		static uint16_t counter;
//		static uint16_t crcValue;
		static uint8_t	cNum;
		

		switch(mAppState)
		{
			case STATE_WAIT:											/*�ȴ�modbus��������*/
				cNum = UsartRead(2,rxFrame);
				if( cNum > 0)
				{
						mAppState = STATE_FRAME_CHECK;
				}
				break;
			case STATE_FRAME_CHECK:								/*���modbus����Ϸ���*/
				if(crcCheck(rxFrame,cNum))
				{
						mAppState = STATE_FRAME_ADDR;
				}
				else
						mAppState = STATE_WAIT;
				break;
			case STATE_FRAME_ADDR:								/*��modbus�����ǲ�������Լ�*/
				if(rxFrame[0] == SLAVE_ADDRESS)
						mAppState = STATE_FRAME_FUNC;
				else
						mAppState = STATE_WAIT;
				break;
			case STATE_FRAME_FUNC:								/*����mobbus����*/
				modbusFunction(rxFrame,cNum);
				mAppState = STATE_WAIT;
				break;
			default:break;
		}
}

/**
  * @brief  crcCheck���ӵ���frame�ǲ�����ȷ�� ע��num����С��2��
  * @param  None
  * @retval 0����ȷ 1������
  */
uint8_t crcCheck(uint8_t * pD, uint8_t num)
{
		uint16_t temp;
		uint16_t crc;

		if(num < 2)	/*numֵС��2 �ᵼ�����鷴Խ�� Ȼ��hartware fault�ж�*/
				return 1;
		
		crc=Get_Crc16(pD,(num-2));
		
		temp = pD[num-2]<<8;
		temp = pD[num-1];
		if(crc == temp)
			return 0;
		else
			return 1;
}
/**
  * @brief  modbusFunction���ݲ�ͬ�Ĺ�����ִ����Ӧ�Ĳ���
	* 				��һ��Ҫ�����ڸ��µ�ֵ���д���
						�ڶ���Ҫ��input���д��� ��Ҫ��ü�ֵ ��ü�ֵ�Ժ�Ҫô�Կؼ����ݽ��мӼ�����
									Ҫô�Ѽ�ֵ�����ֵ����Ϊ״̬���ṩ����
						��ֵ��key       key-value
										��				0x00
										ȷ����		0x01
										ȡ����		0x02
										������    0x03
										����    0x04
										ʱ���		0x05
										�Զ���		0x06
										user��		0x07
										root��		0x08
										deve��		0x09
  * @param  None
  * @retval None
  */		
void modbusFunction(uint8_t * pD, uint8_t num)
{
		uint16_t startAddress;
		uint16_t numOfChannel;
		uint8_t i,j,u8Tmp;
		uint16_t u16Tmp;
		uint32_t u32Tmp;
		uint16_t crc;
		uint8_t moduleCounter;
	
		switch(pD[1])
		{
			/*������Ϊ04����� Ҳ���Ǵ��ݱ������*/
			case 0x04:
				startAddress = (pD[2]<<8)|pD[3];	/*02 03 ������ʼ��ַ*/
				numOfChannel = (pD[4]<<8)|pD[5];	/*04 05 ����ͨ������ �����ж��ٸ�����*/
				txFrame[0] = pD[0];								/*00 ��ַ��*/
				txFrame[1] = pD[1];								/*01 ������*/
				txFrame[2] = numOfChannel*2;			/*�ֽ���*/
				
				/*����������*/

				txFrame[3] = MvcGetTransation().cardStatus.serialNum[0];
				txFrame[4] = MvcGetTransation().cardStatus.serialNum[1];
				txFrame[5] = MvcGetTransation().cardStatus.serialNum[2];
				txFrame[6] = MvcGetTransation().cardStatus.serialNum[3];
				txFrame[7] = (MvcGetTransation().cardStatus.rxWalletValue) >> 24;
				txFrame[8] = (MvcGetTransation().cardStatus.rxWalletValue) >> 16;
				txFrame[9] = (MvcGetTransation().cardStatus.rxWalletValue) >> 8;
				txFrame[10] = (MvcGetTransation().cardStatus.rxWalletValue);
				txFrame[11] = 0;
				txFrame[12] = MvcGetTransation().chargeMode;
				txFrame[13] = (MvcGetTransation().chargeTime)>>24;
				txFrame[14] = (MvcGetTransation().chargeTime)>>16;
				txFrame[15] = (MvcGetTransation().chargeTime)>>8;
				txFrame[16] = (MvcGetTransation().chargeTime);
				txFrame[17] = (MvcGetTransation().chargeMoney)>>24;
				txFrame[18] = (MvcGetTransation().chargeMoney)>>16;
				txFrame[19] = (MvcGetTransation().chargeMoney)>>8;
				txFrame[20] = (MvcGetTransation().chargeMoney);
				txFrame[21] = (MvcGetTransation().chargeElectricity)>>24;
				txFrame[22] = (MvcGetTransation().chargeElectricity)>>16;
				txFrame[23] = (MvcGetTransation().chargeElectricity)>>8;
				txFrame[24] = (MvcGetTransation().chargeElectricity);
				txFrame[25] = (MvcGetTransation().accumulateTime)>>24;
				txFrame[26] = (MvcGetTransation().accumulateTime)>>16;
				txFrame[27] = (MvcGetTransation().accumulateTime)>>8;
				txFrame[28] = (MvcGetTransation().accumulateTime);
				txFrame[29] = (MvcGetTransation().accumulateMoney)>>24;
				txFrame[30] = (MvcGetTransation().accumulateMoney)>>16;
				txFrame[31] = (MvcGetTransation().accumulateMoney)>>8;
				txFrame[32] = (MvcGetTransation().accumulateMoney);
				txFrame[33] = (MvcGetTransation().accumulateElectricity)>>24;
				txFrame[34] = (MvcGetTransation().accumulateElectricity)>>16;
				txFrame[35] = (MvcGetTransation().accumulateElectricity)>>8;
				txFrame[36] = (MvcGetTransation().accumulateElectricity);
				txFrame[37] = 0;
				txFrame[38] = MvcGetTransation().transactionStatus;
				txFrame[39] = MvcGetTransation().transactionSponsor[0];
				txFrame[40] = MvcGetTransation().transactionSponsor[1];
				txFrame[41] = MvcGetTransation().transactionSponsor[2];
				txFrame[42] = MvcGetTransation().transactionSponsor[3];
				txFrame[43] = MvcGetTransation().transactionPayer[0];
				txFrame[44] = MvcGetTransation().transactionPayer[1];
				txFrame[45] = MvcGetTransation().transactionPayer[2];
				txFrame[46] = MvcGetTransation().transactionPayer[3];
			  //txFrame[47] = adc3Value()>>8;
				//txFrame[48] = adc3Value();
				txFrame[47] = 0;				//������״̬
				txFrame[48] = MvcGetTransation().CCstate;
				txFrame[49] = 0;
				txFrame[50] = MvcGetTransation().rate * 100;
				txFrame[51] = 0;
				txFrame[52] = MvcGetTransation().moneyOK;
				txFrame[53] = 0;
				txFrame[54] = MvcGetTransation().matchStatus;
				txFrame[55] = 0;
				txFrame[56] = MvcGetTransation().lockStatus;
				//txFrame[57] = adc1Value()>>8;//0;
				//txFrame[58] = adc1Value();//*pTemperature;
				txFrame[57] = 0;
				txFrame[58] = *pTemperature;
				txFrame[59] = 0;
				txFrame[60] = *pHumidity;
				txFrame[61] = 0;
				txFrame[62] = MvcGetTransation().chargeStatus;
				txFrame[63] = 0;								/*����ָ��*/
				txFrame[64] = MvcGetActiveState();
				
				txFrame[65] = 0;
				txFrame[66] = *piDM100ECurrent;
				txFrame[67] = 0;
				txFrame[68] = *piDM100EVoltage;
				txFrame[69] = (MvcGetTransation().w)>>24;
				txFrame[70] = (MvcGetTransation().w)>>16;
				txFrame[71] = (MvcGetTransation().w)>>8;
				txFrame[72] = (MvcGetTransation().w);
				txFrame[73] = 0;
				txFrame[74] = MvcGetTransation().cardStatus.status;

				txFrame[75] = (MvcGetTransation().rtcTime.year)>>8;
				txFrame[76] = (MvcGetTransation().rtcTime.year);
				txFrame[77] = 0;
				txFrame[78] = (MvcGetTransation().rtcTime.month);
				txFrame[79] = 0;
				txFrame[80] = (MvcGetTransation().rtcTime.day);
				txFrame[81] = 0;
				txFrame[82] = (MvcGetTransation().rtcTime.hour);
				txFrame[83] = 0;
				txFrame[84] = (MvcGetTransation().rtcTime.minute);
				txFrame[85] = 0;
				txFrame[86] = (MvcGetTransation().rtcTime.second);
				
				txFrame[87] = (MvcGetTransation().startTime.year)>>8;
				txFrame[88] = (MvcGetTransation().startTime.year);
				txFrame[89] = 0;
				txFrame[90] = (MvcGetTransation().startTime.month);
				txFrame[91] = 0;
				txFrame[92] = (MvcGetTransation().startTime.day);
				txFrame[93] = 0;
				txFrame[94] = (MvcGetTransation().startTime.hour);
				txFrame[95] = 0;
				txFrame[96] = (MvcGetTransation().startTime.minute);
				txFrame[97] = 0;
				txFrame[98] = (MvcGetTransation().startTime.second);
				
				txFrame[99] = (MvcGetTransation().endTime.year)>>8;
				txFrame[100] = (MvcGetTransation().endTime.year);
				txFrame[101] = 0;
				txFrame[102] = (MvcGetTransation().endTime.month);
				txFrame[103] = 0;
				txFrame[104] = (MvcGetTransation().endTime.day);
				txFrame[105] = 0;
				txFrame[106] = (MvcGetTransation().endTime.hour);
				txFrame[107] = 0;
				txFrame[108] = (MvcGetTransation().endTime.minute);
				txFrame[109] = 0;
				txFrame[110] = (MvcGetTransation().endTime.second);
				txFrame[111] = 0;
				txFrame[112] = 0;
				txFrame[113] = 0;
				txFrame[114] = 0;
				
				/*
				txFrame[17] = adc3Value()>>8;
				txFrame[18] = adc3Value();
				txFrame[19] = adc1Value()>>8;
				txFrame[20] = adc1Value();
				*/
				
				crc=Get_Crc16(txFrame,115);			/*����ҪУ��ĸ���*/
				txFrame[115] = (crc&0xff00)>>8;	/*�����ɵ�У���� ������Ӧ���*/
				txFrame[116] = crc&0xff;
				
				UsartWrite(2,txFrame,117);
			
				break;
			case 0x05:
				/*������Ϊ05����� Ҳ���Ǵ����ֵ�����*/
				UsartWrite(2,pD,num);
				if(pD[4] == 0xff)		/*����*/
					keyValue =  pD[3];
				else								/*̧��*/
					/**************/;
				break;
			case 0x10:
				/*������Ϊ16����� Ҳ��ֻд�����*/
				txFrame[0] = pD[0];								/*00 ��ַ��*/
				txFrame[1] = pD[1];								/*01 ������*/
				txFrame[2] = pD[2];								/*�ֽ���*/
				txFrame[3] = pD[3];								/*�ֽ���*/
				txFrame[4] = pD[4];								/*ͨ����*/
				txFrame[5] = pD[5];								/*ͨ����*/
				crc=Get_Crc16(txFrame,6);			/*����ҪУ��ĸ���*/
				txFrame[6] = (crc&0xff00)>>8;	/*�����ɵ�У���� ������Ӧ���*/
				txFrame[7] = crc&0xff;
				UsartWrite(2,txFrame,8);
			
				/*���ݴ���*/
				u32Tmp = (pD[7]<<24)|(pD[8]<<16)|(pD[9]<<8)|(pD[10]);
				MvcSetChargeValue(u32Tmp);
				break;
			default:break;
		}
}

/**
  * @brief  ��ü�ֵ �����֮��ͽ���ֵ���� Ҳ����û�а������µ�״̬
						��ʵ���ǰ�����Ϊ�а������º�û�а�����������״̬
						�а������¾Ͷ�ȡ��ֵ
						������ʱ�䳤���ַ�Ϊ�����ͳ����Ͷ̰�����״̬ �ⲻ������
						������ʱ�����
						��ֵ��key       key-value
										��				0x00
										ȷ����		0x01
										ȡ����		0x02
										������    0x05
										����    0x0
										ʱ���		0x0
										�Զ���		0x0
										user��		0x04
										root��		0x03
										deve��		0x09
  * @param  None
  * @retval None
  */
uint8_t GetKeyValue(void)
{
		uint8_t temp;
	
		temp = keyValue;
		keyValue = 0;			/*����*/
		return temp;
}