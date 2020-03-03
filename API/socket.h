/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOCKET_H
#define __SOCKET_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
/* Exported macro ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
void Timer2_Init_Config(void);		//Timer2��ʼ������
void System_Initialization(void);	//STM32ϵͳ��ʼ������(��ʼ��STM32ʱ�Ӽ�����)
void W5500Delay(unsigned int d);			//��ʱ����(ms)
int W5500Init(void);
void W5500Task(void);
#endif
