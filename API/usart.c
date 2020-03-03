/**
  ******************************************************************************
  * @file    USART/Interrupt/main.c 
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
#include "usart.h"
#include "string.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_Interrupt
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define MAX_TX_BUFFER_SIZE 150	//���ͻ�������ʵ��ֵ ��������ᵼ�´���
#define MAX_RX_BUFFER_SIZE 150   //���ܻ�������ʵ��ֵ ��������ᵼ�´���
/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;

uint8_t usart1TxCounter = 0;
uint8_t usart2TxCounter = 0;
uint8_t usart3TxCounter = 0;
uint8_t usart4TxCounter = 0;

uint8_t usart1RxCounter = 0;
uint8_t usart2RxCounter = 0;
uint8_t usart3RxCounter = 0;
uint8_t usart4RxCounter = 0;


struct txBuffer
{
	uint8_t queue[MAX_TX_BUFFER_SIZE];	//����һ������������
	uint8_t size;		//ʵ���ֽڸ���
	uint8_t ready;		//0������û��׼���� 1������׼������
							//�жϸ��ݴ˱�־λ���ܾ����Ƿ���
}usart1TxBuffer, usart2TxBuffer, usart3TxBuffer, usart4TxBuffer;

struct rxBuffer
{
	uint8_t queue[MAX_RX_BUFFER_SIZE];	//����һ������������
	uint8_t size;		//ʵ���ֽڸ���
	uint8_t ready;		//0������û��׼���� 1������׼������
							//�жϸ��ݴ˱�־λ���ܾ����Ƿ���
}usart1RxBuffer, usart2RxBuffer, usart3RxBuffer, usart4RxBuffer;

/* Private function prototypes -----------------------------------------------*/
void RCC_Usart_Configuration(void);
void GPIO_Usart_Configuration(void);
void NVIC_Usart_Configuration(void);

void Delay(u32 i)
{
	for(;i>0;i--);
} 
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   usartInit program
  * @param  None
  * @retval None
  */
int usartInit()
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System Clocks Configuration */
  RCC_Usart_Configuration();
       
  /* NVIC configuration */
  NVIC_Usart_Configuration();

  /* Configure the GPIO ports */
  GPIO_Usart_Configuration();

/* USARTy and USARTz configuration ------------------------------------------------------*/
  /* USARTy and USARTz configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
	
	 /* Configure USARTy (USART 1) */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTy, &USART_InitStructure);
	
  /* Configure USARTz (USART 2)*/
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTz, &USART_InitStructure);
	/* Configure USARTa (USART 3)*/
	USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTa, &USART_InitStructure);
	/* Configure USARTb (USART 4)*/
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	//USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_Even;
	//USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTb, &USART_InitStructure);
  
  /* Enable USARTy Receive and Transmit interrupts */
  USART_ITConfig(USARTy, USART_IT_RXNE, ENABLE);
//  USART_ITConfig(USARTy, USART_IT_TXE, ENABLE);

  /* Enable USARTz Receive and Transmit interrupts */
  USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE); //ֻҪrx buffer��Ϊ�վ��ж�
 // USART_ITConfig(USARTz, USART_IT_TXE, ENABLE);	//ֻҪtx bufferΪ�վ��ж�
 
   /* Enable USARTa Receive and Transmit interrupts */
  USART_ITConfig(USARTa, USART_IT_RXNE, ENABLE); //ֻҪrx buffer��Ϊ�վ��ж�
 // USART_ITConfig(USARTa, USART_IT_TXE, ENABLE);	//ֻҪtx bufferΪ�վ��ж�
 
   /* Enable USARTb Receive and Transmit interrupts */
  USART_ITConfig(USARTb, USART_IT_RXNE, ENABLE); //ֻҪrx buffer��Ϊ�վ��ж�
 // USART_ITConfig(USARTb, USART_IT_TXE, ENABLE);	//ֻҪtx bufferΪ�վ��ж�

  /* Enable the USARTy */
  USART_Cmd(USARTy, ENABLE);
  /* Enable the USARTz */
  USART_Cmd(USARTz, ENABLE);
	/* Enable the USARTa */
  USART_Cmd(USARTa, ENABLE);
	/* Enable the USARTb */
  USART_Cmd(USARTb, ENABLE);
	
	usart1TxBuffer.ready = 1;		/*����տ�ʼ��ʱ����Է���*/
	usart2TxBuffer.ready = 1;
	usart3TxBuffer.ready = 1;
	usart4TxBuffer.ready = 1;
	
	usart1RxBuffer.ready = 0;		/*����տ�ʼ��ʱ���ܽ���*/
	usart2RxBuffer.ready = 0;
	usart3RxBuffer.ready = 0;
	usart4RxBuffer.ready = 0;
  return 0;

}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Usart_Configuration(void)
{   
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(USARTy_GPIO_CLK | USARTz_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(USARTa_GPIO_CLK | USARTb_GPIO_CLK | USARTc_GPIO_TX_CLK | USARTc_GPIO_RX_CLK |RCC_APB2Periph_AFIO, ENABLE);

  /* Enable USARTy Clock */
  RCC_APB2PeriphClockCmd(USARTy_CLK, ENABLE); 
  /* Enable USARTz Clock */
  RCC_APB1PeriphClockCmd(USARTz_CLK, ENABLE);  
	/* Enable  usart3 4 5 Clock*/
	RCC_APB1PeriphClockCmd(USARTa_CLK, ENABLE); 
	RCC_APB1PeriphClockCmd(USARTb_CLK, ENABLE); 
	RCC_APB1PeriphClockCmd(USARTc_CLK, ENABLE); 
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Usart_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USARTy Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTy_RxPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTy_GPIO, &GPIO_InitStructure);
  
  /* Configure USARTz Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTz_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);  
	
	/* Configure USARTa Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTa_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTa_GPIO, &GPIO_InitStructure);  
	
	/* Configure USARTb Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTb_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTb_GPIO, &GPIO_InitStructure);
  
  /* Configure USARTy Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTy_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTy_GPIO, &GPIO_InitStructure);

  /* Configure USARTz Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTz_TxPin;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);  
	
	/* Configure USARTa Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTa_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTa_GPIO, &GPIO_InitStructure);
	
	/* Configure USARTb Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTb_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTb_GPIO, &GPIO_InitStructure);
		
	/*485 con configure */
#ifdef USART2_485
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_9);	/*485 ��̬Ϊ����*/
#endif

#ifdef USART3_485
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_7);		/*485 ��̬Ϊ����*/
#endif

#ifdef USART4_485
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_7);		/*485 ��̬Ϊ����*/
#endif

}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Usart_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTy_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTa_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTb_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}

/**
  * @brief  �жϷ��ͺ��� Ϊ�˲�ռ��CPU��Դ �������жϷ��� ���������ڱ���Ҫ�������
						�ڴ˺���������while�ȴ�����Ȼʱ�䲻�᳤������Ҳ�ǿ��ܵ�ϵͳ��������
						Ҫô�Ѵ˺�������ʵʱ���̣�Ҫô������Ϣ���Ʋ�ͣ���
						�������ʵʱ���̣���ô��̫��ʵ������Ե��á�
						���������Ϣ���������һЩ
  * @param  None
  * @retval None
  */
uint8_t UsartWrite(uint8_t UsartNum, uint8_t * pD , uint8_t size)
{
		switch(UsartNum)
		{
			case 1:	while(usart1TxBuffer.ready == 0){};
							memcpy(usart1TxBuffer.queue,pD,size);
							usart1TxBuffer.size = size;
							USART_ITConfig(USARTy, USART_IT_TXE, ENABLE);
							break;
			case 2:	while(usart2TxBuffer.ready == 0){};
							memcpy(usart2TxBuffer.queue,pD,size);
							usart2TxBuffer.size = size;
							USART_ITConfig(USARTz, USART_IT_TXE, ENABLE);
							break;
			case 3:	while(usart3TxBuffer.ready == 0){};
							memcpy(usart3TxBuffer.queue,pD,size);
							usart3TxBuffer.size = size;
							USART_ITConfig(USARTa, USART_IT_TXE, ENABLE);
							break;
			case 4:	while(usart4TxBuffer.ready == 0){};
							memcpy(usart4TxBuffer.queue,pD,size);
							usart4TxBuffer.size = size;
							USART_ITConfig(USARTb, USART_IT_TXE, ENABLE);
							break;
			default:break;
		}
		
		return 0;
}

/**
  * @brief  ���ڽ��պ��� ��ʱ����ɨ��
  * @param  None
  * @retval None
  */
uint8_t UsartRead(uint8_t UsartNum, uint8_t * pD)
{
		uint8_t numOfByte;
		switch(UsartNum)
		{
			case 1:
						if(usart1RxBuffer.ready == 1)
						{
									memcpy(pD,usart1RxBuffer.queue,usart1RxBuffer.size);
									numOfByte = usart1RxBuffer.size;
									usart1RxBuffer.ready = 0;
									usart1RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			case 2:
						if(usart2RxBuffer.ready == 1)
						{
									memcpy(pD,usart2RxBuffer.queue,usart2RxBuffer.size);
									numOfByte = usart2RxBuffer.size;
									usart2RxBuffer.ready = 0;
									usart2RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			case 3:
						if(usart3RxBuffer.ready == 1)
						{
									memcpy(pD,usart3RxBuffer.queue,usart3RxBuffer.size);
									numOfByte = usart3RxBuffer.size;
									usart3RxBuffer.ready = 0;
									usart3RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			case 4:
						if(usart4RxBuffer.ready == 1)
						{
									memcpy(pD,usart4RxBuffer.queue,usart4RxBuffer.size);
									numOfByte = usart4RxBuffer.size;
									usart4RxBuffer.ready = 0;
									usart4RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			default:break;
		}
}
			
				
/**
  * @brief  �������������� �����Դ��� ��ʱ��� 
  * @param  None
  * @retval None
  */
void usartStateMachine()
{
	uint8_t linkEstablished = 0;
	//���յ�ʱ����Ϣ timeBase++
	
	//���յ�������Ϣ
	USART_ITConfig(USARTy, USART_IT_TXE, ENABLE);
	linkEstablished = 1;
	
	/*���յ��ⲿ��Ϣ
	if(linkEstablished == 1)
		//����ʱ�� �����ǲ��ǳ�ʱ��
	else
		//ֱ�Ӷ��� ������
		
	//ʱ�䴦��
	if(linkEstablished == 1)
		//�����ǲ��ǳ�ʱ��
	else
		//����            */
}

void usart1SendByte(uint8_t byte)
{
	USART_ClearFlag(USARTy, USART_FLAG_TC);	//��λ��1 ���Ҫ����
	USART_SendData(USARTy, byte);
	while(USART_GetFlagStatus(USARTy,USART_FLAG_TC) != SET);
}

/**
  * @brief  modbus �շ�������
  * @param  none
  * @retval None
	* @detail �˺���ÿ10msִ��һ�� ��Ҫ�����û���յ����� Ȼ��������� ��������
	* ��������������һ�ű�� ֮���ڽ������ ��ʵ�������Ӧ�÷ŵ��߲�����������
	* ������Ͳ����зֲ���
  */
void MbTransceiver(struct message data)
{
		/*���ȼ����ܻ����ready�Ƿ�Ϊ1 ���������б��Ľ���*/
		/*������� ���ʾû�����ݵ��� ��ʲôҲ����*/
//			unsigned char* pv=(unsigned char*)&Display_VALUE_V;
//			unsigned char* pa=(unsigned char*)&Display_VALUE_A;
//			unsigned char* pw=(unsigned char*)&Display_VALUE_KW;
//			signed char* ptem=(signed char*)&temperature;
//			unsigned char* phum=(unsigned char*)&humidity;
	
			uint8_t rsp_info_crc_frame[29]={0};
			uint8_t rsp_info_frame[27]={0};
			uint16_t Crc;
			
		if(usart2RxBuffer.ready == 0)
			return;
		
		/*usart2RxBuffer*/
		if((usart2RxBuffer.queue[1]==0x02)&&(usart2RxBuffer.queue[6]==0xf9)&&(usart2RxBuffer.queue[7]==0xcb))
		{
//				USART2_Write(rsp_status_frame,6);
			 
				/*USART2 �򿪽����ж�*/
				USART_ITConfig(USART2,USART_IT_RXNE ,ENABLE);
		}
		else if((usart2RxBuffer.queue[1]==0x04)&&(usart2RxBuffer.queue[6]==0x00)&&(usart2RxBuffer.queue[7]==0x0f))
		{
				/*---------------------����Ļ���͵�֡-----------------------*/
				/*floatת16����*/	
		 
//				pv=(unsigned char*)&Display_VALUE_V;
//				rsp_info_frame[7]=pv[3];
//				rsp_info_frame[8]=pv[2];
//				rsp_info_frame[9]=pv[1];
//				rsp_info_frame[10]=pv[0];
				/*��ѹ*/
				
//				pa=(unsigned char*)&Display_VALUE_A;
//				rsp_info_frame[11]=pa[3];
//				rsp_info_frame[12]=pa[2];
//				rsp_info_frame[13]=pa[1];
//				rsp_info_frame[14]=pa[0];
				/*����*/
		
//				pw=(unsigned char*)&Display_VALUE_KW;
//				rsp_info_frame[15]=pw[3];
//				rsp_info_frame[16]=pw[2];
//				rsp_info_frame[17]=pw[1];
//				rsp_info_frame[18]=pw[0];
				/*����*/
		
		
				/*�¶�*/
//				ptem=(signed char*)&temperature;
//				rsp_info_frame[19]=ptem[3];
//				rsp_info_frame[20]=ptem[2];
//				rsp_info_frame[21]=ptem[1];
//				rsp_info_frame[22]=ptem[0];
	
				/*ʪ��*/
//				phum=(unsigned char*)&humidity;
//				rsp_info_frame[23]=phum[3];
//				rsp_info_frame[24]=phum[2];
//				rsp_info_frame[25]=phum[1];
//				rsp_info_frame[26]=phum[0];
		
//				for(i=0;i<27;i++)
				{
//					rsp_info_crc_frame[i]=rsp_info_frame[i];
				}
//				Crc=Get_Crc16(rsp_info_frame,27);
		
				/*��16λ��crcд��*/
				rsp_info_crc_frame[27]=(Crc&0xff00)>>8;
				rsp_info_crc_frame[28]=Crc&0xff;
				 
		
//				USART2_Write(rsp_info_crc_frame,29);
		}
		/*�����������0x03����Ϊ��ȡ���Ľ��*/
		else if((usart2RxBuffer.queue[1]==0x03)&&(usart2RxBuffer.queue[6]==0xf4)&&(usart2RxBuffer.queue[7]==0x08))
		{
		
//				for(i=0;i<7;i++)
				{
//						rsp_read_money_crc_frame[i]=rsp_read_money_frame[i];
				}
//				Crc=Get_Crc16(rsp_read_money_frame,7);
//				rsp_read_money_crc_frame[7]=(Crc&0xff00)>>8;
//				rsp_read_money_crc_frame[8]=Crc&0xff;
				
//				USART2_Write(rsp_read_money_crc_frame,9);
		}
		
		/*���¿�ʼ��磬�򿪼̵���*/
		//��Ȼ�Ǹ�����Ļ��������֡ �ظ���ͬ֡ �����һֱ�� ͨѶ��������
		else if((usart2RxBuffer.queue[1]==0x05)&&(usart2RxBuffer.queue[4]==0xFF))
	  {
				/*ԭ֡����*/
	//			USART2_Write(on_rsp,8);

    }

		/*������ֹ��磬�رռ̵���*/
		else if((usart2RxBuffer.queue[1]==0x05)&&(usart2RxBuffer.queue[4]==0x00))
		{
				GPIO_SetBits(GPIOE,GPIO_Pin_4);	
		//		USART2_Write(off_rsp,8);
		}
		/*���·��ؼ����߰��·���������*/
		else if((usart2RxBuffer.queue[1]==0x10)&&(usart2RxBuffer.queue[11]==0xB3)&&(usart2RxBuffer.queue[12]==0xef))
		{
		//		USART2_Write(mainwindow,8);
		}
}

void USART2Write(uint8_t *Data, uint8_t len)
{
		uint8_t counter;
		/*�ȴ�readyΪ1 ֻҪ�����жϴ� readyһ�����Ա�1*/
		while(usart2TxBuffer.ready == 0);
		
		for(counter = 0; counter < len; counter++)
			//	usart2TxBuffer.queue[counter] = data[counter];
		
		usart2TxBuffer.ready = 1;		/*���������Ѿ����*/
		usart2TxBuffer.size = len;
	
		USART_ITConfig(USARTz, USART_IT_TXE, ENABLE);	/*���ж� �Զ�����*/
}

void Usart2SetReady()
{
		usart2RxBuffer.ready = 1;
}
void Usart3SetReady()
{
		usart3RxBuffer.ready = 1;
}
void Usart4SetReady()
{
		usart4RxBuffer.ready = 1;
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

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles USARTy global interrupt request.
  * @param  None
  * @retval None
  */
void USARTy_IRQHandler(void)
{
		if(USART_GetITStatus(USARTy, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTy, USART_IT_RXNE);
				
				if(usart1RxBuffer.size == 0)	/*���������û������*/
				{
						usart1RxBuffer.ready = 0;	/*���ڵĻ��崦�ڲ���״̬*/
						//vUsart2TimerSetAlarm(2);	/*��ʱ����ʼ��ʱ*/
				}
				
				/* Read one byte from the receive data register */
				/*��������ж��������ǰû���ü������� �������һ��FIFO*/
				usart1RxBuffer.queue[usart1RxCounter++] = USART_ReceiveData(USARTy);
				
				usart1RxBuffer.size = usart1RxCounter;	/*����size size�������*/
				
				//vUsart2TimerSetAlarm(2);	/*��ʱ����ʼ��ʱ*/
				
					/* Disable the USARTz Receive interrupt */
				//USART_ITConfig(USARTz, USART_IT_RXNE, DISABLE);

		}
		
		if(USART_GetITStatus(USARTy, USART_IT_TXE) != RESET)
		{   
#ifdef USART1_485
			//GPIO_SetBits(GPIOG,GPIO_Pin_9);
#endif
			usart1TxBuffer.ready = 0;
			if(usart1TxCounter == usart1TxBuffer.size) /*�˴β�û������Ҫ�� ֻ��Ϊ�˹��ж�*/
			{	
					/* Disable the USARTy Transmit interrupt */
					USART_ClearITPendingBit(USARTy, USART_IT_TXE);
					USART_ITConfig(USARTy, USART_IT_TXE, DISABLE);
				
#ifdef USART1_485
				//	GPIO_ResetBits(GPIOG,GPIO_Pin_9);
#endif		
					usart1TxBuffer.ready = 1;
					usart1TxCounter = 0;		//���������һ�μ���
			}
			else
				/* Write one byte to the transmit data register д�벻�ȴ� */
				USART_SendData(USARTy, usart1TxBuffer.queue[usart1TxCounter++]);
		}
}

/**
  * @brief  This function handles USARTz global interrupt request.
  * @param  None
  * @retval None
  */
void USARTz_IRQHandler(void)
{
		if(USART_GetITStatus(USARTz, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTz, USART_IT_RXNE);
				
				if(usart2RxBuffer.size == 0)	/*���������û������*/
				{
						usart2RxCounter = 0;			/*���������*/
						usart2RxBuffer.ready = 0;	/*���ڵĻ��崦�ڲ���״̬*/
						vUsart2TimerSetAlarm(3);	/*��ʱ����ʼ��ʱ*/
				}
				
				/* Read one byte from the receive data register */
				/*��������ж��������ǰû���ü������� �������һ��FIFO*/
				usart2RxBuffer.queue[usart2RxCounter++] = USART_ReceiveData(USARTz);
				
				usart2RxBuffer.size = usart2RxCounter;	/*����size size�������*/
				
				vUsart2TimerSetAlarm(3);	/*��ʱ����ʼ��ʱ*/
		}
		
		if(USART_GetITStatus(USARTz, USART_IT_TXE) != RESET)
		{   
			USART_ClearITPendingBit(USARTz, USART_IT_TXE);
#ifdef USART2_485
			GPIO_SetBits(GPIOG,GPIO_Pin_9);
#endif
			usart2TxBuffer.ready = 0;
			
			if(usart2TxCounter == usart2TxBuffer.size) /*�˴β�û������Ҫ�� ֻ��Ϊ�˹��ж�*/
			{	
					/* Disable the USARTz Transmit interrupt */
					USART_ITConfig(USARTz, USART_IT_TXE, DISABLE);
					USART_ClearITPendingBit(USARTz, USART_IT_TXE);
				
          USART_ITConfig(USARTz, USART_IT_TC, ENABLE);	/*������������ж�*/
				
					usart2TxCounter = 0;		//���������һ�μ���
					usart2TxBuffer.ready = 1;	/*����ȷʵ�Ѿ����Բ����� ��ʵ�ʻ���һ���ֽ��ڷ�����*/
			}
			else
				/* Write one byte to the transmit data register д�벻�ȴ� */
			{
					USART_SendData(USARTz, usart2TxBuffer.queue[usart2TxCounter++]);
			}
		}
		
		if (USART_GetITStatus(USARTz, USART_IT_TC) != RESET) 
		{
				USART_ClearITPendingBit(USARTz, USART_IT_TC);
				USART_ITConfig(USARTz, USART_IT_TC, DISABLE);
#ifdef USART2_485
				GPIO_ResetBits(GPIOG,GPIO_Pin_9);
#endif
		}
}

/**
  * @brief  This function handles USARTa global interrupt request.
  * @param  None
  * @retval None
  */
void USARTa_IRQHandler(void)
{
		if(USART_GetITStatus(USARTa, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTa, USART_IT_RXNE);

				if(usart3RxBuffer.size == 0)	/*���������û������*/
				{
						usart3RxCounter = 0;			/*���������*/
						usart3RxBuffer.ready = 0;	/*���ڵĻ��崦�ڲ���״̬*/
						vUsart3TimerSetAlarm(3);	/*��ʱ����ʼ��ʱ*/
				}
				
				/* Read one byte from the receive data register */
				/*��������ж��������ǰû���ü������� �������һ��FIFO*/
				usart3RxBuffer.queue[usart3RxCounter++] = USART_ReceiveData(USARTa);

				usart3RxBuffer.size = usart3RxCounter;	/*����size size�������*/
				
				vUsart3TimerSetAlarm(3);	/*��ʱ����ʼ��ʱ*/
		}
		
		if(USART_GetITStatus(USARTa, USART_IT_TXE) != RESET)
		{   
			USART_ClearITPendingBit(USARTa, USART_IT_TXE);
#ifdef USART3_485
			GPIO_SetBits(GPIOG,GPIO_Pin_8);
#endif
			usart3TxBuffer.ready = 0;
			
			if(usart3TxCounter == usart3TxBuffer.size) /*�˴β�û������Ҫ�� ֻ��Ϊ�˹��ж�*/
			{	
					/* Disable the USARTy Transmit interrupt */
					USART_ITConfig(USARTa, USART_IT_TXE, DISABLE);
					USART_ClearITPendingBit(USARTa, USART_IT_TXE);
				
          USART_ITConfig(USARTa, USART_IT_TC, ENABLE);	/*������������ж�*/
				
					usart3TxCounter = 0;		//���������һ�μ���
					usart3TxBuffer.ready = 1;	/*����ȷʵ�Ѿ����Բ����� ��ʵ�ʻ���һ���ֽ��ڷ�����*/
			}
			else
				/* Write one byte to the transmit data register д�벻�ȴ� */
			{
					USART_SendData(USARTa, usart3TxBuffer.queue[usart3TxCounter++]);
			}
		}
		
		if (USART_GetITStatus(USARTa, USART_IT_TC) != RESET) 
		{
				USART_ClearITPendingBit(USARTa, USART_IT_TC);
				USART_ITConfig(USARTa, USART_IT_TC, DISABLE);
#ifdef USART3_485
				GPIO_ResetBits(GPIOG,GPIO_Pin_8);
#endif
		}
}

/**
  * @brief  This function handles USARTb global interrupt request.
  * @param  None
  * @retval None
  */
void USARTb_IRQHandler(void)
{
    if(USART_GetITStatus(USARTb, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTb, USART_IT_RXNE);
				//usart1SendByte(0xaa);
				if(usart4RxBuffer.size == 0)	/*���������û������*/
				{
						usart4RxCounter = 0;			/*���������*/
						usart4RxBuffer.ready = 0;	/*���ڵĻ��崦�ڲ���״̬*/
						vUsart4TimerSetAlarm(4);	/*��ʱ����ʼ��ʱ*/
				}
				
				/* Read one byte from the receive data register */
				/*��������ж��������ǰû���ü������� �������һ��FIFO*/
				usart4RxBuffer.queue[usart4RxCounter++] = USART_ReceiveData(USARTb);

				usart4RxBuffer.size = usart4RxCounter;	/*����size size�������*/
				
				vUsart4TimerSetAlarm(4);	/*��ʱ����ʼ��ʱ*/
		}
		
		if(USART_GetITStatus(USARTb, USART_IT_TXE) != RESET)
		{   
			USART_ClearITPendingBit(USARTb, USART_IT_TXE);
#ifdef USART4_485
			GPIO_SetBits(GPIOG,GPIO_Pin_7);
#endif
			usart4TxBuffer.ready = 0;
			
			if(usart4TxCounter == usart4TxBuffer.size) /*�˴β�û������Ҫ�� ֻ��Ϊ�˹��ж�*/
			{	
					/* Disable the USARTy Transmit interrupt */
					USART_ITConfig(USARTb, USART_IT_TXE, DISABLE);
					USART_ClearITPendingBit(USARTb, USART_IT_TXE);
				
          USART_ITConfig(USARTb, USART_IT_TC, ENABLE);	/*������������ж�*/
				
					usart4TxCounter = 0;		//���������һ�μ���
					usart4TxBuffer.ready = 1;	/*����ȷʵ�Ѿ����Բ����� ��ʵ�ʻ���һ���ֽ��ڷ�����*/
			}
			else
			/* Write one byte to the transmit data register д�벻�ȴ� */
			{
					USART_SendData(USARTb, usart4TxBuffer.queue[usart4TxCounter++]);
			}
		}
		
		if (USART_GetITStatus(USARTb, USART_IT_TC) != RESET) 
		{
				USART_ClearITPendingBit(USARTb, USART_IT_TC);
				USART_ITConfig(USARTb, USART_IT_TC, DISABLE);
#ifdef USART4_485
				GPIO_ResetBits(GPIOG,GPIO_Pin_7);
#endif
		}
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
