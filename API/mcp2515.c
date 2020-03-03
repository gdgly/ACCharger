/**
  ******************************************************************************
  * @file    mcp2515.c 
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
#include "message.h"
#include "usart.h"
#include "mcp2515.h"
#include "spi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  
  * @param  
  * @retval 
  */
void Mcp2515Init(void)
{
		uint8_t temp[4] = {0,0,0,0};
		
		/*GPIO init:
						 CS:	GPIOG 14 output
						INT:	GPIOE 5  input
		*/
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOE,&GPIO_InitStructure);

		
		/* MCP2515 �����λ */
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RESET);
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	 
		/*�ʵ���ʱ ��ʼ�����ý���*/
		Delay(0xffffff);
		
		//λָ��MCP2515 ��������ģʽ
		//CANCTRL��REQOP[2:0]��Ϊ100
		Mcp2515BitModify(CANCTRL, 0xE0, (1<<REQOP2));
		Delay(65535);
	 	
	 //���㲢����MCP2515��λʱ��
	 
	 //  ʱ��Ƶ��   Fosc  = 16MHz
	 //  ��Ƶ������ CNF1.BRP[5:0] = 7
	 //  ��Сʱ��ݶ� TQ = 2 * ( BRP + 1 ) / Fosc   = 2*(7+1)/16M = 1uS
	 //  ͬ���� Sync Seg   = 1TQ
	 //  ������ Prop Seg   = ( PRSEG + 1 ) * TQ  = 1 TQ
	 //  ��λ����� Phase Seg1 = ( PHSEG1 + 1 ) * TQ = 3 TQ
	 //  ��λ����� Phase Seg2 = ( PHSEG2 + 1 ) * TQ = 3 TQ
	 //  ͬ����ת��������Ϊ CNF1.SJW[1:0] = 00,  1TQ
	 //  ���߲����� NBR = Fbit =  1/(sync seg + Prop seg + PS1 + PS2 )
	 //                       = 1/(8TQ) = 1/8uS = 125kHz
	 
	 //???????CNF1.BRP[5:0] = 7,????????? CNF1.SJW[1:0] = 00
	 Mcp2515WriteByte( CNF1, (1<<BRP0)|(1<<BRP1)|(1<<BRP2) );
	 // ????? Prop Seg ?00,?1TQ,????? Phase Seg1???3TQ
	 Mcp2515WriteByte( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	 // ?? ????? Phase Seg2? 3TQ , ???????
	 Mcp2515WriteByte( CNF3, (1<<PHSEG21) );
	 
	 
	 
	 //����Ϊ 500kbps ,TQ = 1/8us
	 //CNF1.BRP[5:0] = 0,????????? CNF1.SJW[1:0] = 01
	// Mcp2515WriteByte( CNF1, (1<<BRP0)|(1<<SJW0) );    // 500kbps
	 //Mcp2515WriteByte( CNF1, (1<<SJW0) );               //1Mbps
		// Prop Seg 00,1TQ, Phase Seg1 3TQ
	 //Mcp2515WriteByte( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	 // Phase Seg2  3TQ , 
	 //Mcp2515WriteByte( CNF3, (1<<PHSEG21) );
	 
	 
	 //MCP2515
	// Mcp2515WriteByte( CANINTE, /*(1<<RX1IE)|(1<<RX0IE)*/ 0 );
	 
	 //�����жϿ����� ʹ��MCP2515���ջ������ж�
	 Mcp2515WriteByte( CANINTE, (1<<RX1IE)|(1<<RX0IE) );
	 
	 
	 // �رս��ջ�����0���˲����� RXM[1:0]=11, �������б���
	 Mcp2515WriteByte( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
	 
	 // �رս��ջ�����1���˲����� RXM[1:0]=11 �������б���
	 Mcp2515WriteByte( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );
	 
	 //����6�������˲��Ĵ���Ϊ0
	 Mcp2515WriteArray( RXF0SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF1SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF2SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF3SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF4SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF5SIDH, temp, 4 );
	 
	 //����2�������˲��Ĵ���Ϊ0
	 Mcp2515WriteArray( RXM0SIDH, temp, 4 );
	 Mcp2515WriteArray( RXM1SIDH, temp, 4 );
	 
	 //���ý�������������üĴ��� ��ֹ�ڶ�����
	 Mcp2515WriteByte(BFPCTRL, 0);
	 
	 //����ʹ�� ����BFPCTRL��RX0BF,RX1BFΪ�������
	 Mcp2515BitModify( BFPCTRL, (1<<B1BFE)|(1<<B0BFE)|(1<<B1BFM)|(1<<B0BFM), (1<<B1BFE)|(1<<B0BFE) );
	 
	 
	 //���÷�������������üĴ��� ��ֹ�ڶ�����
	 Mcp2515WriteByte(TXRTSCTRL, 0);
	 
	 
	 //MCP2515����ģʽ ���빦�ܲ���
	 //Mcp2515BitModify( CANCTRL, 0XE0, (1<<REQOP1) );
	 
	 //MCP2515��������ģʽ
	  Mcp2515BitModify(CANCTRL, 0xE0, 0);
		Delay(65535);
		
		Mcp2515WriteByte(CANINTF,0);
}


/**
  * @brief  Mcp2515WriteByteд���ƼĴ���
  * @param  
  * @retval 
  */
void Mcp2515WriteByte(uint8_t address, uint8_t data)
{
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
 
	SPIByte(SPIz, SPI_WRITE);
	SPIByte(SPIz, address);
	SPIByte(SPIz, data);
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  Mcp2515ReadByte�����ƼĴ���
  * @param  
  * @retval 
  */
uint8_t Mcp2515ReadByte(uint8_t address)
{
	uint8_t data;
 
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
	SPIByte(SPIz, SPI_READ); 
	SPIByte(SPIz, address); 
	data = SPIByte(SPIz, 0xff); 
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
 
	return data;
}

/**
  * @brief  Mcp2515ReadRxBuffer�����ջ���
  * @param  
  * @retval 
  */
uint8_t Mcp2515ReadRxBuffer(uint8_t address)
{
		uint8_t data;
	
		if (address & 0xF9)
			return 0;
 
		// CS low 
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
 
		SPIByte(SPIz, SPI_READ_RX | address); 
		data = SPIByte(SPIz, 0xff);
 
		//CS high 
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
 
		return data;
}

/**
  * @brief  Mcp2515BitModifyλ�޸�
  * @param  
  * @retval 
  */
void Mcp2515BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
	SPIByte(SPIz, SPI_BIT_MODIFY); 
	SPIByte(SPIz, address);    
	SPIByte(SPIz, mask);     
	SPIByte(SPIz, data);    
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  Mcp2515WriteArray �������Ĵ�����������д����
  * @param  
  * @retval 
  */
void Mcp2515WriteArray( uint8_t address, uint8_t *data, uint8_t length )
{
		uint8_t i;
 
		// CS low 
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	 
		SPIByte(SPIz, SPI_WRITE);
		SPIByte(SPIz, address);  
		for (i=0; i<length ;i++ )
			SPIByte(SPIz, *data++);  
	 
		//CS high
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  Mcp2515ReadArray �������Ĵ�����������������
  * @param  
  * @retval 
  */
void Mcp2515ReadArray( uint8_t address, uint8_t *data, uint8_t length )
{
	uint8_t i;
 
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
 
	SPIByte(SPIz, SPI_READ); 
	SPIByte(SPIz, address);  
	for (i=0; i<length ;i++ )
		*data++ = SPIByte(SPIz, 0xff);  
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  SendCANmsg ����can���� ����Ƚϼ� ֻ�Ƿ��� ���漰���� �����ж�
  * @param  
  * @retval 
  */
void sendCANmsg()
{
		Mcp2515BitModify(TXB0CTRL, 0x08, 0x00);
	
		Mcp2515WriteByte(TXB0SIDH,0x01);
		Mcp2515WriteByte(TXB0SIDL,0xe0);
	
		Mcp2515WriteByte(TXB0DLC, 8);
	
		Mcp2515WriteByte(TXB0D0, 1);
		Mcp2515WriteByte(TXB0D1, 2);
		Mcp2515WriteByte(TXB0D2, 3);
		Mcp2515WriteByte(TXB0D3, 4);
		Mcp2515WriteByte(TXB0D4, 5);
		Mcp2515WriteByte(TXB0D5, 6);
		Mcp2515WriteByte(TXB0D6, 7);
		Mcp2515WriteByte(TXB0D7, 8);
	
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz, SPI_READ);
		SPIByte(SPIz, 0x30);
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
		SPIByte(SPIz,SPI_RTS|0x01);
		
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/** Send a CAN message
* \param bi transmit buffer index 0, 1 ,2
* \param id message identifier
* \param data pointer to data to be stored
* \param prop message properties, the octet has following structure:
* - bits 7:6 - message priority (higher the better)
* - bit 5 - if set, message is remote request (RTR)
* - bit 4 - if set, message is considered to have ext. id.
* - bits 3:0 - message length (0 to 8 bytes) */
/* EID17 ----- EID0 ��18λ  SID10 ------SID0 ��11λ  18+11=29 */
void SendCANmsg(uint8_t bi,
								unsigned long id,
								uint8_t * data,
								uint8_t prop)
{
		uint8_t i;
		uint32_t SID;
		uint32_t EID;
		
		/*Ĭ������� id�������� SID + EID ��λ�ڸ�λ*/
		/*�˺�����   id�������� EID + SID ��λ�ڸ�λ*/
		SID = id >> 18;
		EID = id & 0x03ffff;
		id = (EID << 11)|SID;
	
		/* Initialize reading of the receive buffer */
		Mcp2515BitModify(TXBnCTRL(bi), 0x08, 0x00);
	
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		/* Send header and address */
		SPIByte(SPIz, SPI_WRITE);
		SPIByte(SPIz, TXBnCTRL(bi));
	
		/* Setup message priority */
		SPIByte(SPIz,prop >> 6);		/*ֻ��67λ*/
	
		/* Setup standard or extended identifier */
		/* std �ȷ� Ȼ�� ext�ٷ� ���ȷ��͵Ĳ�һ��*/
		/* EID17 ----- EID0 ��18λ  SID10 ------SID0 ��11λ  18+11=29 */
		if(prop & 0x10)				/*��չ֡*/
		{
				SPIByte(SPIz,(uint8_t)(id>>3));
				SPIByte(SPIz,(uint8_t)(id<<5)|(1<<EXIDE)|(uint8_t)(id>>27));
				SPIByte(SPIz,(uint8_t)(id>>19));
				SPIByte(SPIz,(uint8_t)(id>>11));
		} 
		else 								 /*��׼֡*/	
		{
				SPIByte(SPIz,(uint8_t)(id>>3));
				SPIByte(SPIz,(uint8_t)(id<<5)|(0<<EXIDE)|(uint8_t)(id>>27));
				SPIByte(SPIz,(uint8_t)(id>>19));
				SPIByte(SPIz,(uint8_t)(id>>11));
		}
		
		/* Setup message length and RTR bit */
		SPIByte(SPIz,(prop & 0x0F) | ((prop & 0x20) ? (1 << RTR) : 0));
		
		/* Store the message into the buffer */
		for(i = 0; i < (prop & 0x0F); i++)
			SPIByte(SPIz,data[i]);
		
		/* Release the bus */
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
		
		/* Send request to send */
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz, SPI_READ);
		SPIByte(SPIz, 0x30);
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
		
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RTS| (1 << bi));
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
* Example code - simple CAN listener.
*
*/
#define getData(n) msgReceived[6+i];
#define getId (unsigned short)((msgReceived[1]<<3)|(msgReceived[2]>>5));
#define getLength msgReceived[5] >> 4;
#define setRollover(v) Mcp2515BitModify(RXB0CTRL, 1 << BUKT, v << BUKT);
#define getMode (Mcp2515ReadByte(CANSTAT) >> 5);
#define setMode(mode) { Mcp2515BitModify(CANCTRL, (7 << REQOP0), \
									(mode << REQOP0)); while(getMode != mode); }

unsigned char * msgReceived = (void *)0;
									
						
uint8_t rbuffer[2][14]; /* 2 RX buffers, each have 14B */
uint8_t ready[2];				/* ������ݽ������*/
									
/**
  * @brief  ��ý���״̬ ����1rx ���� 2rx
  * @param  
  * @retval 
  */						
uint8_t getRXState()
{
		uint8_t rxStatus;
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RX_STATUS);
		rxStatus = SPIByte(SPIz,0x00);
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
		return rxStatus;
}

/* get receive buffer index (we don't consider that both buffer
contain message, this situation in our environment cannot happen �
message is directly copied from the buffer and released in IRQ)*/
void Mcp2515Receive(void)
{
		uint8_t i;
		uint8_t bi = getRXState() >> 6;
	
		/* Copy the message from the device and release buffer */
	
		if(GPIO_ReadOutputDataBit(GPIOE,5) == SET)
			return;
				
		//usart1SendByte(Mcp2515ReadByte(CANINTF));
	
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
		SPIByte(SPIz,SPI_READ_RX);
		SPIByte(SPIz,RXBnCTRL(bi));
		/* Make the local copy */
		for(i = 0; i < 14; i++)
				rbuffer[bi][i] = SPIByte(SPIz,0x00);
	
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	
		msgReceived = rbuffer[bi];
		
		//usart1SendByte(Mcp2515ReadByte(CANINTF));
		
		for(i = 0; i < 14; i++)
				usart1SendByte(rbuffer[bi][i]);
}

/**
  * @brief  MCP2515Task��Ҫ����mcp2515�յ�������,
  * @param  
  * @retval 
  */
#define STATE_WAIT					0
#define STATE_INT_LOW				1
#define STATE_INT_HIGH			2

static uint8_t mcpAppState = STATE_WAIT;    // State tracking variable

void MCP2515Task(void)
{
		static uint8_t i;
		static uint8_t bi;

    switch(mcpAppState)
    {
				case STATE_WAIT:
						bi = 0;
						mcpAppState = STATE_INT_HIGH;
						break;
				
				case STATE_INT_HIGH:
						if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5) == RESET)	/*������:INT�͵�ƽ*/
						{		
								mcpAppState = STATE_INT_LOW;
						}
						break;
		
        case STATE_INT_LOW:	
						/*�����ж����ĸ�����01 10 11�������*/
						bi = getRXState() >> 6;
		
						if(bi == 0x00)
						{
								mcpAppState = STATE_WAIT;
								break;
						}
						else if(bi == 0x01 || bi == 0x10)
						{
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*ʹ��SPI_READ_RX �����Զ������ر�־λ*/
								SPIByte(SPIz,SPI_READ_RX | ((bi>>1)<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*�ڽ��ܵ���ͬʱ ��һ����ǽ��м�¼ ����״̬*/
										/*һ��������ֻ������״̬ ���յ��� û�н��ܵ�*/
										rbuffer[(bi>>1)][i] = SPIByte(SPIz,0x00);
								}
								ready[(bi>>1)] = 1;		/*�Ѿ����յ���*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*����SPI_READ_RX����Ͳ���Mcp2515WriteByte(CANINTF,0);��*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[(bi>>1)][i]);
						}
						else
						{
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*ʹ��SPI_READ_RX �����Զ������ر�־λ*/
								SPIByte(SPIz,SPI_READ_RX | (0<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*�ڽ��ܵ���ͬʱ ��һ����ǽ��м�¼ ����״̬*/
										/*һ��������ֻ������״̬ ���յ��� û�н��ܵ�*/
										rbuffer[0][i] = SPIByte(SPIz,0x00);
								}
								ready[0] = 1;		/*�Ѿ����յ���*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*����SPI_READ_RX����Ͳ���Mcp2515WriteByte(CANINTF,0);��*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[0][i]);
								
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*ʹ��SPI_READ_RX �����Զ������ر�־λ*/
								SPIByte(SPIz,SPI_READ_RX | (1<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*�ڽ��ܵ���ͬʱ ��һ����ǽ��м�¼ ����״̬*/
										/*һ��������ֻ������״̬ ���յ��� û�н��ܵ�*/
										rbuffer[1][i] = SPIByte(SPIz,0x00);
								}
								ready[1] = 1;		/*�Ѿ����յ���*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*����SPI_READ_RX����Ͳ���Mcp2515WriteByte(CANINTF,0);��*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[1][i]);
						}
				
						mcpAppState = STATE_WAIT;
						break;
				default:break;
		}
}

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

/**
  * @brief  MCP2515Read()��ȡ���ܵ������� ���жϽ��ܵ�û�� ����ȡ����
  * @param  
  * @retval 
  */
uint8_t MCP2515Read(CanTo485 * frame)
{
		if(ready[0] == 1)
		{
				
		}
		else if(ready[1] == 1)
		{
				
		}
}