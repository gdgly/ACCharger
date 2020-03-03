/**
  ******************************************************************************
  * @file    CAN/Networking/main.c 
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
#include "string.h"
#include "can.h"
#include "usart.h"
#include "pgn.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup CAN_Networking
  * @{
  */ 


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define __CAN1_USED__
/* #define __CAN2_USED__*/

#ifdef  __CAN1_USED__
  #define CANx                       CAN1
  #define GPIO_CAN                   GPIO_CAN1
  #define GPIO_Remapping_CAN         GPIO_Remapping_CAN1
  #define GPIO_CAN                   GPIO_CAN1
  #define GPIO_Pin_CAN_RX            GPIO_Pin_CAN1_RX
  #define GPIO_Pin_CAN_TX            GPIO_Pin_CAN1_TX
#else /*__CAN2_USED__*/ 
  #define CANx                       CAN2
  #define GPIO_CAN                   GPIO_CAN2
  #define GPIO_Remapping_CAN             GPIO_Remap_CAN2
  #define GPIO_CAN                   GPIO_CAN2
  #define GPIO_Pin_CAN_RX            GPIO_Pin_CAN2_RX
  #define GPIO_Pin_CAN_TX            GPIO_Pin_CAN2_TX
#endif  /* __CAN1_USED__ */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
CAN_InitTypeDef        CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
CanTxMsg TxMessage;
pgnGroup pgnValue;			/*�������� ��gpn.c���Ѿ�����extern��*/

struct canQueueTx canTxBuf;
struct canQueueRx canRxBuf;
struct message canMessageTx;	//can�����õ���Ϣʵ��
struct message canMessageRx;	//can�����õ���Ϣʵ��

/* Private function prototypes -----------------------------------------------*/
void NVIC_CAN_Config(void);
void CAN_Config(void);
void LED_Display(uint8_t Ledstatus);
void Init_RxMes(CanRxMsg *RxMessage);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  canInit program.
  * @param  None
  * @retval None
  */
int canInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* NVIC configuration */
  NVIC_CAN_Config();
   
  /* CAN configuration */
  CAN_Config();
  
  CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
	
	CanQueueInit();
 
  /* Transmit example
  TxMessage.StdId = 0x321;
  TxMessage.ExtId = 0x01;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 1;
  TxMessage.Data[0] = 0xaa;				//д������
  CAN_Transmit(CANx, &TxMessage);		//���͹��̣�����*/
}

/**
  * @brief  Configures the CAN.
  * @param  None
  * @retval None
  */
void CAN_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* GPIO clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
#ifdef  __CAN1_USED__
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN1, ENABLE);
#else /*__CAN2_USED__*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN2, ENABLE);
#endif  /* __CAN1_USED__ */
  /* Configure CAN pin: RX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_RX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_CAN, &GPIO_InitStructure);
  
  /* Configure CAN pin: TX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_TX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_CAN, &GPIO_InitStructure);
  
//  GPIO_PinRemapConfig(GPIO_Remapping_CAN , ENABLE);
  
  /* CANx Periph clock enable */
#ifdef  __CAN1_USED__
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
#else /*__CAN2_USED__*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
#endif  /* __CAN1_USED__ */
  
  
  /* CAN register init ��ȫ����ʼ��Ϊ0����λ��ʼ����*/
  CAN_DeInit(CANx);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;	//�¼�����ͨ��ģʽ �ر�
  CAN_InitStructure.CAN_ABOM = ENABLE;	//�Զ����߹���     �ر�
  CAN_InitStructure.CAN_AWUM = ENABLE; //�Զ�����ģʽ	   �ر�
  CAN_InitStructure.CAN_NART = DISABLE; //��ֹ�����Զ��ش� �ر�
  CAN_InitStructure.CAN_RFLM = DISABLE; //����FIFO����ģʽ �ر�
  CAN_InitStructure.CAN_TXFP = DISABLE; //����FIFO���ȼ�   �ر�
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//��������ģʽ
  
  /* CAN Baudrate = 1MBps*/
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;//����ͬ����Ծ���
  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;//ʱ���1
  CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;//ʱ���2
  CAN_InitStructure.CAN_Prescaler = 4;	  //�����ʷ�Ƶ��
  CAN_Init(CANx, &CAN_InitStructure);

  /* CAN filter init */
#ifdef  __CAN1_USED__
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
#else /*__CAN2_USED__*/
  CAN_FilterInitStructure.CAN_FilterNumber = 14;
#endif  /* __CAN1_USED__ */
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
}

/**
  * @brief  Configures the NVIC for CAN.
  * @param  None
  * @retval None
  */
void NVIC_CAN_Config(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
#ifndef STM32F10X_CL
#ifdef  __CAN1_USED__
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
#else  /*__CAN2_USED__*/
  /* CAN2 is not implemented in the device */
   #error "CAN2 is implemented only in Connectivity line devices"
#endif /*__CAN1_USED__*/
#else
#ifdef  __CAN1_USED__ 
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
#else  /*__CAN2_USED__*/
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
#endif /*__CAN1_USED__*/

#endif /* STM32F10X_CL*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Initializes a Rx Message.  Ĭ�� CAN_ID_EXT
  * @param  CanRxMsg *RxMessage  
  * @retval None
  */
void Init_RxMes(CanRxMsg *RxMessage)
{
  uint8_t i = 0;

  RxMessage->StdId = 0x00;
  RxMessage->ExtId = 0x00;
  RxMessage->IDE = CAN_ID_EXT;
  RxMessage->DLC = 0;
  RxMessage->FMI = 0;
  for (i = 0;i < 8;i++)
  {
    RxMessage->Data[i] = 0x00;
  }
}

/**
  * @brief  CanTxTaskֻ���������� �������ж��н��� ���Բ��ø���
						���ǵ�һ�� TX���˺���
											 RX���ж�
  * @param  None
  * @retval None
  */
void CanTxTask()
{
		CanTxMsg txMessage;				/*can������Ϣ*/
		if(CanQueueTxDequeue(&txMessage) == 0)
		{
				/*��������ʵ���ǵ�һ��*/
				CAN_Transmit(CANx, &txMessage);
				/*���ݷ���ֵ�����ж��Ƿ���Է��ͳ�ȥ*/
				//..
		}
}

#define STATE_WAIT					0
#define STATE_RECV					1
#define STATE_SEND					2
static uint8_t cAppState = STATE_WAIT;    // State tracking variable
/**
  * @brief  CanTask״̬��ʵʱ���� ���ǽ���--����--���� ��ͣ�ɼ�FIFO���ݣ��ڶ��㣩
	���������������1����ͨ8�ֽ����� �Ǿ�����ͨ������
									2������Э�� ��ʱ���ձ�Ϊһ�����̣�Ҫ�������ӣ��������ݣ�ʱ�����
	�ڽ��ܹ����иı�������Ҫ������
									1��PGN������ �����������Ŀ��
									2��linkstatus�� �����Ϊ�˹��������õ�
	����ֻ��һ�������һ����Ҫ���͵�ʱ����Ϊ�˴���Э�����Ҫ��������linkstatus��
										ȷ������ʲô��ע�⣬�˴��ķ���ֻ��Ϊ�˴���Э��
	��ô����˵���������������ǵڶ���������·��Ĺ��ܣ����պͷ��Ͷ��ǻ���linkstatus
	�����еģ�����һ�㣬�����ǳ�֡���Ƕ�֡������˵��ֻ�Ǵ���֡��ʽ��������ƾ�����
	�ݡ������ڽ����д���PGN�������ǲ����ʵġ�Ӧ���ټ�һ�㣬����һ����Ҫ����������ݡ�
	��Ŀǰ����ʱ���ô˲�����ṹ��
	���⣬��������·���У���Ҫ������յ���֡��һ����¼�������¼��Ҫ������յ���ʲô
	���͵ı��ġ��Թ��ⲿʹ�á�������һ�����У������н��յ��ı��ļ�¼һ�£���״̬����
	��״̬�л���
  * @param  None
  * @retval None
  */
void CanTask()
{
	static CanRxMsg rxMessage;				/*can������Ϣ*/
	static CanTxMsg txMessage;				/*can������Ϣ*/
	
	switch(cAppState)
	{
		case STATE_WAIT:								/*��ʼ��*/
			Init_RxMes(&rxMessage);
			cAppState = STATE_RECV;
			break;
		case STATE_RECV:		/*����Ľ����ǳ�������Ľ��� �Ǹ���canlink�������ݾ�����*/
			if(CanQueueRxDequeue(&rxMessage) == 0)
			{
					/*�������ǵڶ���  �������� ʱ�����*/
					canRxHandle(rxMessage);	
			}
			cAppState = STATE_SEND;
			break;
		case STATE_SEND:		/*����ķ����ǳ�������ķ��� �Ǹ���canlink�������ݾ�����*/
			canTxHandle();
			cAppState = STATE_WAIT;
			break;
		defalult:break;
	}
}

/*Э�鴫��� �˱��¼��������Э�鴫����صĲ��� ���в������ǻ��ڴ˱���е�*/
struct linkStatus canLink;

/**
  * @brief  canRxHandle (�ڶ���)������ͨ���ݺ�Э�鴫�����ݵ����ݣ���ͨ���ݾ��Ǹ���
		������Э�鴫�����ݾ��Ǵ������ӣ�Ȼ����²�����ͬʱ��Ҫ֪ͨ״̬����
		�������ϵͳ��Ϊ״̬Ǩ���ṩ������
		�����ַ����� 1.����Ϣ֪ͨ
								 2.����conditon��API
  * @param  CanRxMsg
  * @retval None
  */
void canRxHandle(CanRxMsg rxMessage)
{
		uint32_t pgn;
	
		/*������ͨ��� (��Ӧ�ǵ���������)*/
		NormalMessageParse(rxMessage);
	
		/*����Э�鴫�����*/
		pgn = (rxMessage.ExtId >> 16) & 0x000000ff;     /*��ID����ȡpgn*/
		switch(pgn)
		{
			case 0xEC:											/*����Э��*/
				if(rxMessage.Data[0] == 0x10)	/*RTS*/
				{
						canLink.RTSReceived = 1;
						canLink.numOfByte = (rxMessage.Data[2]<<8) | rxMessage.Data[1];
						canLink.numOfPackage = rxMessage.Data[3];
						canLink.PGNCodeLow = rxMessage.Data[5];
						canLink.PGNCode = rxMessage.Data[6];
						canLink.PGNCodeHigh = rxMessage.Data[7];
						/*�򿪶�ʱ��*/
						vCanTimerSetAlarm(200);
				}
				break;
			case 0xE8:
				/*�����ж����ӽ�����û�� linkestablish ����data[0]���뵽���� ���Ҵ򿪶�ʱ��*/
				if(canLink.linkEstablished == 1)
				{
						canLink.buffer[rxMessage.Data[0]].available = 1;
						memcpy(canLink.buffer[rxMessage.Data[0]].data,rxMessage.Data,8);
						/*�򿪶�ʱ��*/
						vCanTimerSetAlarm(200);
				}
				break;
			default:break;
		}
}
/**
  * @brief  canTxHandle����linkstatus��� ʵʱ���� ɨ�輸��״̬ ��if else��ʽ�ж�
	�Ե��е��� �Ժ���Ըý��ɱ��ʽ���
  * @param  None
  * @retval None
  */
void canTxHandle()
{
		uint8_t counter;		/*��ͨ������*/
		CanTxMsg txMessage;
	
		/*�����Ӳ�δ�����������*/
		if(canLink.linkEstablished == 0)
		{
				if(canLink.RTSReceived == 1)
				{
						txMessage.ExtId = 0x1CECF455;
						txMessage.RTR = CAN_RTR_DATA;
						txMessage.IDE = CAN_ID_EXT;
						txMessage.DLC = 8;
						txMessage.Data[0] = 0x11;			/*�����ֽ�*/
						txMessage.Data[1] = canLink.numOfPackage;			/*�ɷ������ݰ���*/
						txMessage.Data[2] = 0x01;			/*�������͵ı��*/
						txMessage.Data[3] = 0xff;
						txMessage.Data[4] = 0xff;
						txMessage.Data[5] = canLink.PGNCodeLow;
						txMessage.Data[6] = canLink.PGNCode;
						txMessage.Data[7] = canLink.PGNCodeHigh;
						
						/*����CTS*/
						CanQueueTxEnqueue(txMessage);	/*Ͷ�����׼������*/
						canLink.CTSSended = 1;
					
						/*���ӽ���*/
						canLink.linkEstablished = 1;
						vCanTimerSetAlarm(200);				/*��ʼ��ʱ200ms �����ʱ �����ж�*/
				}
		}
		else			/*�����Ѿ������������*/
		{
				if(canLink.emSended == 1)
				{
						LinkClose();					/*�ر�����*/
				}
				else
				{
						for(counter = 0; counter < canLink.numOfPackage; counter++)
						{
								if(canLink.buffer[counter].available == 0)
										break;
						}
						/*ɨ�����е������Ƿ��յ�*/
						/*û�յ����˳� �յ��ͷ���em*/
						if(counter == canLink.numOfPackage)			/*ȫ���յ�*/
						{
								txMessage.ExtId = 0x1CECF455;
								txMessage.RTR = CAN_RTR_DATA;
								txMessage.IDE = CAN_ID_EXT;
								txMessage.DLC = 8;
								txMessage.Data[0] = 0x13;			/*�����ֽ�*/
								txMessage.Data[1] = canLink.numOfByte;			/*�ɷ������ݰ���*/
								txMessage.Data[2] = canLink.numOfByte >> 8;			/*�������͵ı��*/
								txMessage.Data[3] = canLink.numOfPackage;
								txMessage.Data[4] = 0xff;
								txMessage.Data[5] = canLink.PGNCodeLow;
								txMessage.Data[6] = canLink.PGNCode;
								txMessage.Data[7] = canLink.PGNCodeHigh;
								CanQueueTxEnqueue(txMessage);	/*Ͷ�����׼������*/
								
								vCanTimerClose();						/*��ʱ�ر�*/
							
								canLink.emSended = 1;
							
								/*�����ݲɼ���PGN����� (��Ӧ�ǵ���������)*/
								//...
								LongMessageParse(&canLink);
						}
				}
		}
}
/**
  * @brief  LinkClose�ر����� ����������� 
  * @param  None
  * @retval None
  */
void LinkClose()
{
		memset(&canLink,0,sizeof(canLink));		/*�����������Ӳ���*/
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @brief  CanQueueInit��ʼ�����ͺͽ��ն��� �������ж��� ����ȫ���������� ������
	����û�л������� �����������
  * @param  None
  * @retval None
  */
void CanQueueInit()
{
		canTxBuf.front = 0;			/*���ͻ��� ����ͻ��Զ�����*/
		canTxBuf.rear = 0;
												
		canRxBuf.front = 0;			/*���ջ��� ����Ѿ����ܵ�*/
		canRxBuf.rear = 0;
}
/**
  * @brief  ֻ���ж��е��� ֻ���ж��и���rearֵ
  * @param  None
  * @retval None
  */
uint8_t CanQueueRxEnqueue(CanRxMsg data)		
{
	uint8_t temp;
	if((((canRxBuf.rear) + 1)%CAN_BUFFER_SIZE) == canRxBuf.front)		//����
	{
			return 1;
	}
	else
	{
			temp = canRxBuf.rear;
			canRxBuf.canBuffer[temp] = data;
			canRxBuf.rear = (temp + 1) % CAN_BUFFER_SIZE;
			return 0;
	}
}
/**
  * @brief  ֻ�ڳ����е��� ֻ�ڳ������и���frontֵ
  * @param  None
  * @retval None
  */
uint8_t CanQueueRxDequeue(CanRxMsg * data_pointer)
{
		uint8_t temp;
		if(canRxBuf.front == canRxBuf.rear)							//����
		{
  			return 1;
		}
		else
		{
  			temp = canRxBuf.front;
				*data_pointer = canRxBuf.canBuffer[temp];
				canRxBuf.front = (temp + 1)%CAN_BUFFER_SIZE;
				return 0;
		}
}
/**
  * @brief  ֻ���ж��е��� ֻ�ں����и���rearֵ
  * @param  None
  * @retval None
  */
uint8_t CanQueueTxEnqueue(CanTxMsg data)		
{
	uint8_t temp;
	if((((canTxBuf.rear) + 1)%CAN_BUFFER_SIZE) == canTxBuf.front)		//����
	{
			return 1;
	}
	else
	{
			temp = canTxBuf.rear;
			canTxBuf.canBuffer[temp] = data;
			canTxBuf.rear = (temp + 1) % CAN_BUFFER_SIZE;
			return 0;
	}
}
/**
  * @brief  ֻ�ڳ����е��� ֻ�ڳ������и���frontֵ
  * @param  None
  * @retval None
  */
uint8_t CanQueueTxDequeue(CanTxMsg * data_pointer)
{
		uint8_t temp;
		if(canTxBuf.front == canTxBuf.rear)							//����
		{
  			return 1;
		}
		else
		{
  			temp = canTxBuf.front;
				*data_pointer = canTxBuf.canBuffer[temp];
				canTxBuf.front = (temp + 1)%CAN_BUFFER_SIZE;
				return 0;
		}
}
/**
  * @}
  */

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

CanRxMsg RxMessage;

/**
  * @brief  This function handles CAN1 Handler.
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
		//ֻҪ����FIFO�������� �ͻ᲻ͣ������ж� û�и�CPU�������㹻ʱ�䴦����Ϣ
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage); /*�������ж��ж�ȡ����*/
		CanQueueRxEnqueue(RxMessage);				/*��ȡ��֮��ѹ����� �ȴ�ʵʱ�����ȡ*/
}

/**
  * @brief  This function handles CAN2 Handler.
  * @param  None
  * @retval None
  */
#ifdef STM32F10X_CL
void CAN2_RX0_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
  if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1))
  {
  }
}
#endif


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
