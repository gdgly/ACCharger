#include "spibysoft.h"
#include "string.h"
#include "usart.h"
#include "systick.h"


void SpiBySoftInit()
{
		GPIO_RC522_Configuration();
}
 /*******************************************************************************
* ��  ��  :  ����ʱ�Ӽ�����LED��USART1��RFID��IO��
* ��  ��  :  ��
* ��  ��  :  ��
* ��  ��  :  ��
*******************************************************************************/
void GPIO_RC522_Configuration(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
				 
	  /*SPI��������*/
	 /*PC.6---OUT---MISO*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	 /*PC.7---DIN---MOSI   PC.8---CLK---SCK*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8;   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 /*�������*/
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	 /*PA8---RST---RST */
   GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 /*PC9---CS---SDA*/
	 GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9; 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
	 
}
void NVIC_RC522_Configuration(void)
{

}
/*******************************************************************
@��    �ܣ�SPIд���� ���ڲ���IO�������ݲ��������в������ж� ������
@										 �ж��в��ܶ�SPI����
@��    �룺 ��
@��    �����޷���ֵ
*******************************************************************/ 
void SPIWriteByte(uint8_t num)    
{  
	uint8_t count=0;
	uint8_t i;     
	for(count=0;count<8;count++)  
	{ 	  
			if(num&0x80)
				TRC_DIN_SET(1);
			else 
				TRC_DIN_SET(0);
			num<<=1;    
			TRC_CLK_SET(0);				/*��������Ч */
			for(i=0;i<15;i++);	  /*��ʱ*/
			TRC_CLK_SET(1);
			for(i=0;i<15;i++);		/*��ʱ*/    
	}	    
} 
/*******************************************************************
@��    �ܣ�SPI������  ���ڲ���IO�������ݲ��������в������ж� ������
											�ж��в��ܶ�SPI����
@��    �룺 ��
@��    ��:	 ��
********************************************************************/ 
uint8_t SPIReadByte(void)	  
{ 	 
		uint8_t  SPICount,i;    // Counter used to clock out the data
		uint8_t  SPIData;                  
		SPIData = 0;                 /*�½�����Ч*/
	
		for(SPICount = 0; SPICount < 8; SPICount++) // Prepare to clock in the data to be read
		{
				SPIData <<=1;                     // Rotate the data
			
				TRC_CLK_SET(1); 	
				for(i=0;i<15;i++); 
				TRC_CLK_SET(0); 		
				for(i=0;i<15;i++);               // Raise the clock to clock the data out of the MAX7456
				if(DOUT)
				{
						SPIData|=0x01;
				}   														 // Drop the clock ready for the next bit
		}                                    // and loop back
		return (SPIData);  
}    
/*******************************************************************
@��    �ܣ���RC522�Ĵ��� Ҳ����ĳ��ַ��ֵ
@����˵����Address[IN]:�Ĵ�����ַ
@��    �أ�������ֵ
********************************************************************/
uint8_t ReadRawRC(uint8_t Address)
{
    uint8_t  ucAddr,i;
    uint8_t ucResult=0;
		
    TRC_CS_SET(0);
    ucAddr = ((Address<<1)&0x7E)|0x80;	//���Ĵ�����ʱ�򣬵�ַ���λΪ 1�����λΪ0��1-6λȡ���ڵ�ַ
		SPIWriteByte(ucAddr);
		ucResult=SPIReadByte();
    TRC_CS_SET(1);
		return ucResult;
}

/*******************************************************************
@��    �ܣ�дRC522�Ĵ���	Ҳ����д��ĳ��ַĳֵ
@����˵����Address[IN]:�Ĵ�����ַ
@          value[IN]:д���ֵ
*******************************************************************/
void WriteRawRC(uint8_t  Address, uint8_t  value)
{  
    uint8_t ucAddr,i;

    TRC_CS_SET(0);
    ucAddr = ((Address<<1)&0x7E);	 //д�Ĵ�����ʱ�򣬵�ַ���λΪ 0�����λΪ0��1-6λȡ���ڵ�ַ
		SPIWriteByte(ucAddr);
		SPIWriteByte(value);
    TRC_CS_SET(1);
}

/*******************************************************************
@��    �ܣ���RC522�Ĵ���λ ����λ����
@����˵����reg[IN]:�Ĵ�����ַ
@          mask[IN]:��λֵ
*******************************************************************/
void SetBitMask(uint8_t reg, uint8_t mask)  
{
    int8_t  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/*******************************************************************
@��    �ܣ���RC522�Ĵ���λ
@����˵����reg[IN]:�Ĵ�����ַ
@          mask[IN]:��λֵ
*******************************************************************/
void ClearBitMask(uint8_t reg, uint8_t  mask)  
{
    int8_t  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 
