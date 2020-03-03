/**
  ******************************************************************************
  * @file    CANCOM-100E.c 
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
#include "CANCOM-100E.h"
#include "string.h"
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/*info ID1 ID2 ID3 ID4 data[0] data[1] data[2] data[3] data[4] data[5] data[6] data[7]*/
typedef struct						/*485��CAN�ṹ*/
{
	uint8_t info;
	uint8_t ID1;
	uint8_t ID2;
	uint8_t ID3;
	uint8_t ID4;
	uint8_t data[8];				/*data[0]������ֽ� data[7]������ֽ�*/
}CanTo485;

/* Private define ------------------------------------------------------------*/
#define EXT 0x01
#define STD	0x00
/* Private macro -------------------------------------------------------------*/
#define STATE_WAIT					0
#define STATE_SETTING_WAIT	1
#define STATE_SETTING_TX		2
#define STATE_SETTING_RX		3

/* Private variables ---------------------------------------------------------*/
CanTo485 canComTxFrame;
CanTo485 canComRxFrame;

static uint8_t mAppState = STATE_WAIT;    // State tracking variable

/* Private function prototypes -----------------------------------------------*/
void ModuleSettingMonitor(void);

/* Private functions ---------------------------------------------------------*/						 
/**
  * @brief  FrameTx()�������õ�֡��Ϣ��ID�����ݣ�����һ֡ Ȼ���ͳ�ȥ�����ǲ�û
	*					���жϴ�ʱ��485�Ƿ���working ��ᵼ��һЩ����
  * @param  
  * @retval 
  */
void FrameTx(	uint8_t FF,\
							uint8_t RTR,\
							uint8_t dataLength,\
							uint32_t ID,\
							uint8_t * pD)
{
		uint8_t i;			/*�����������ݳ���*/
		memset(&canComTxFrame,0,sizeof(canComTxFrame));	/*����ȫ����0���*/
	
		canComTxFrame.info = (FF << 7)|(RTR << 6)|(dataLength << 0);
		canComTxFrame.ID4 = ID;
		canComTxFrame.ID3 = ID>>8;
		canComTxFrame.ID2 = ID>>16;
		canComTxFrame.ID1 = ID>>24;
		for(i = 0; i< dataLength; i++)
		{
				canComTxFrame.data[i] = pD[i];
		}
		
		/*���������Ƿ�����*/
		UsartWrite(3,(void *)&canComTxFrame,sizeof(canComTxFrame));
}

/**
  * @brief  MegmeetTask()��ʵʱ������Ҫ��ȡUsart3�����ݣ�Ȼ�����̴���
  * @param  
  * @retval 
  */
void MegmeetTaskOn()
{
		static uint16_t moduleCounter;					/*ģ�������*/
		static uint8_t  CMDCounter;
		static uint8_t	cNum;										/*�ֽڸ���*/

		switch(mAppState)
		{
			case STATE_WAIT:
				moduleCounter = 0;												/*��ʼ��������*/
				CMDCounter = 0;
				mAppState = STATE_SETTING_WAIT;
				vMegmeetTimerSetValue(0);
				break;
			case STATE_SETTING_WAIT:
				if(vMegmeetTimerGetValue() > 500)					/*��ʱ��500ms*/
				{
					mAppState = STATE_SETTING_TX;						/*׼������*/
				}
				break;
			case STATE_SETTING_TX:
			//	MegmeetTx(moduleCounter,2);
				mAppState = STATE_SETTING_RX;
				break;
			case STATE_SETTING_RX:
				cNum = UsartRead(3,(void *)&canComRxFrame);
				if(cNum > 0)
				{
						/*�������Ѿ����յ��� ����megmeet�ظ���֡ �������֡���ݸı��������*/
					
						switch(canComRxFrame.data[0])
						{
							case 2:
								if(canComRxFrame.data[1] != 0x00)
								{
									mAppState = STATE_SETTING_WAIT;
								}
								else
								{
									usart1SendByte(0xee);
									mAppState = STATE_SETTING_WAIT;
								}
								break;
							default:break;
						}
						//....
						
						/*���CMDCounter moduleCounter Ϊ��һ��������׼�� �����϶��Ǳ�����*/
						moduleCounter++;
						if(moduleCounter >= 14)
							moduleCounter = 0;
				}
				else
				{
						moduleCounter++;
						if(moduleCounter >= 14)
							moduleCounter = 0;
						mAppState = STATE_SETTING_WAIT;
				}
				vMegmeetTimerSetValue(0);
				break;
			default:break;
		}
}

/**
  * @brief  MegmeetTask()��ʵʱ������Ҫ��ȡUsart3�����ݣ�Ȼ�����̴���
	*					���ڲ���485��ʽ ����ַ��͵�ʱ����� ���ܵ�ʱ���� �ͻ����
	*					��ˣ�����Ҫ��CAN���й��� ���ҶԷ��͵Ľ�����ƺ�
	*					ÿ200ms��һ��ģ��������������--��ȡ--�������Ȼ����һ��ģ��
	*					��ȻCANͨ�źܿɿ� ����Ҳ���ǻ���û�н��յ�֡�����������Эͬ
	*					��������Ҳ�����л��Ʒ�ֹ����������ĳ��λ�ã�Ŀǰ���ö�ʱ������
  * @param  
  * @retval 
  */
void MegmeetTask()
{
		static uint16_t moduleCounter;					/*ģ�������*/
		static uint8_t  CMDCounter;
		static uint16_t retryCounter;						/*�ش�����*/
		static uint8_t	cNum;										/*�ֽڸ���*/

		switch(mAppState)
		{
			case STATE_WAIT:
				moduleCounter = 0;												/*��ʼ��������*/
				CMDCounter = 0;
				retryCounter = 0;													/*�ش�����*/
				mAppState = STATE_SETTING_WAIT;
				vMegmeetTimerSetValue(0);
				break;
			case STATE_SETTING_WAIT:
				retryCounter = 0;													/*�ش�����*/
				//ModuleSettingMonitor();
				if(vMegmeetTimerGetValue() > 100)					/*��ʱ��100ms*/
				{
					mAppState = STATE_SETTING_TX;						/*׼������*/
				}
				break;
			case STATE_SETTING_TX:
				//MegmeetTx(moduleCounter, CMDCounter);
				vMegmeetTimerSetValue(0);
				mAppState = STATE_SETTING_RX;
				break;
			case STATE_SETTING_RX:
				cNum = UsartRead(3,(void *)&canComRxFrame);
				if(cNum > 0)
				{
						/*�������Ѿ����յ��� ����megmeet�ظ���֡ �������֡���ݸı��������*/
						switch(canComRxFrame.data[0])
						{
							case 0:
								if(canComRxFrame.data[1] != 0x00)
								{
									mAppState = STATE_SETTING_WAIT;
									retryCounter = 0;
								}
								else
								{
									usart1SendByte(moduleCounter);
									usart1SendByte(CMDCounter);
									usart1SendByte(0xee);
									mAppState = STATE_SETTING_TX;
									retryCounter++;
									if(retryCounter == 3)				/*�ش�3�� �����оͺ�����*/
										mAppState = STATE_SETTING_WAIT;
								}
								break;
							case 1:
					//			moduleSetting[moduleCounter].status = (canComRxFrame.data[6]<< 8)|canComRxFrame.data[7];
					//			moduleSetting[moduleCounter].status &= 0xF800;	/*�޹�λ����*/
					//			moduleSetting[moduleCounter].statusVoltage = (canComRxFrame.data[2]<< 8)|canComRxFrame.data[3];
					//			moduleSetting[moduleCounter].statusCurrent = (canComRxFrame.data[4]<< 8)|canComRxFrame.data[5];
								break;
							case 2:
								if(canComRxFrame.data[1] != 0x00)
								{
									mAppState = STATE_SETTING_WAIT;
									retryCounter = 0;
								}
								else
								{
									usart1SendByte(moduleCounter);
									usart1SendByte(CMDCounter);
									usart1SendByte(0xee);
									mAppState = STATE_SETTING_TX;
									retryCounter++;
									if(retryCounter == 3)				/*�ش�3�� �����оͺ�����*/
										mAppState = STATE_SETTING_WAIT;
								}
								break;
							default:break;
						}
						//....
						
						/*���CMDCounter moduleCounter Ϊ��һ��������׼�� �����϶��Ǳ�����*/
						if(retryCounter == 0)	/*��û���ش���״̬�� CMDCounter����+1*/
							CMDCounter++;
						if(CMDCounter == 3)
						{
								CMDCounter = 0; /*����*/
								moduleCounter++;
								if(moduleCounter == MODULENUM)
									moduleCounter = 0;
						}
				}
				else
				{
					if(vMegmeetTimerGetValue() > 50)
					{
						mAppState = STATE_SETTING_WAIT;
					}
				}
				break;
			default:break;
		}
}

/**
  * @brief  MegmeetTx����megmeet�Ĺ涨���ͱ���
  * @param  None
  * @retval 0����ȷ 1������
  */
void MegmeetTx(uint8_t moduleNum, uint8_t CMD, ModuleSetting setting)
{
		uint8_t data[8];
		switch(CMD)
		{
			case 0:
				data[0] = CMD;
				data[1] = 0;
				data[2] = setting.mAcurrent>>8;
				data[3] = setting.mAcurrent;
			
				data[4] = setting.mVvoltage>>24;
				data[5] = setting.mVvoltage>>16;
				data[6] = setting.mVvoltage>>8;
				data[7] = setting.mAcurrent;
				FrameTx(EXT,0,8, 0x1307c080|(ModuleAddr(moduleNum)&0x0f) ,data);
				break;
			case 1:
				data[0] = CMD;
				data[1] = 0;
				data[2] = 0;
				data[3] = 0;
				data[4] = 0;
				data[5] = 0;
				data[6] = 0;
				data[7] = 0;
				FrameTx(EXT,0,8, 0x1307c080|(ModuleAddr(moduleNum)&0x0f) ,data);
				break;
			case 2:
				data[0] = CMD;
				data[1] = 0;
				data[2] = 0;
				data[3] = 0;
				data[4] = 0;
				data[5] = 0;
				data[6] = 0;
				data[7] = setting.on;
				FrameTx(EXT,0,8, 0x1307c080|(ModuleAddr(moduleNum)&0x0f),data);
				break;
			default:break;
		}
}
/**
  * @brief  MegmeetRx���ݲ�ͬCMD ʶ��ظ�
  * @param  None
  * @retval 0����ȷ 1������
  */		
void MegmeetRx(uint8_t moduleNum, uint8_t CMD)
{
	
}

