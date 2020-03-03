#include "AM2301.h"
#include "time.h"
#include "usart.h"

uint8_t data[40];
s16 temperatureValue;
uint16_t crcTemperatureValue;
uint16_t humidityValue;
uint16_t crcHumidityValue;
uint8_t crcValue;
uint8_t crcCaclulate;

/*������ֵ�����ս�� ����д������ȡ�������ⲿ����*/
float temperature;
float humidity;

/*�����߶�ʱ��Ҫ��Ƚϸ� ��Ϣ���ƿ����޷�����
	�������Ϣ���������Эͬ������ Эͬ��������
	��Ϣ���Ƶļ�϶ִ��
*/
#define STATE_WAIT					0
#define STATE_OUTPUT_LOW		1
#define STATE_OUTPUT_HIGH		2
#define STATE_INPUT_LOW			3
#define STATE_INPUT_HIGH		4
#define STATE_DATA_LOW			5
#define STATE_DATA_HIGH			6
#define STATE_DATA_TIME			7
#define STATE_INPUT_DELAY   8
#define STATE_DATA_DELAY_1  9
#define STATE_DATA_DELAY_2  10
static uint8_t gAppState = STATE_WAIT;    // State tracking variable


/****************************************************
@��������AM2301Task �˺�������ռ����timer6����1us��ʱ
					��timer6���ǻ�����ж� 65msһ��
@���룺��
@�������
****************************************************/
void AM2301Task(void)
{
		static uint16_t timeCounter;
		static uint16_t counter;
		static uint8_t	i;
    switch (gAppState)
    {
				case STATE_WAIT:
						i = 0;
						timeCounter = 0;
						counter = 0;
						Tim6SetCounterValue(0);									/*��ʼ��ʱ*/
            gAppState = STATE_OUTPUT_LOW;
            break;
				case STATE_OUTPUT_LOW:
						if(Tim6GetCounterValue() > 64000)				/*�ȴ�60ms ʱ�䵽��*/
						{
								i++;
								if(i == 20)
								{
									i = 0;
									AM2301_OUT_Init();									/*��������Ϊ���*/
									GPIO_ResetBits(GPIOG,GPIO_Pin_11);	/*����*/
									Tim6SetCounterValue(0);							/*��ʼ��ʱ*/
									gAppState = STATE_OUTPUT_HIGH;
								}
								else
									Tim6SetCounterValue(0);
						}
            break;
        case STATE_OUTPUT_HIGH:											
						if(Tim6GetCounterValue() > 25000)				/*�͵�ƽ����25ms ʱ�䵽��*/
						{
								GPIO_SetBits(GPIOG,GPIO_Pin_11);		/*���� ����ʼ�źŽ���*/
								Tim6SetCounterValue(0);							/*��ʼ��ʱ*/
								gAppState = STATE_INPUT_LOW;
						}
            break;
        case STATE_INPUT_LOW:
						if(Tim6GetCounterValue() > 30)			/*�ߵ�ƽ����30us*/
						{
								AM2301_IN_Init();								/*��Ϊ����*/
								Tim6SetCounterValue(0);							/*��ʼ��ʱ*/
								gAppState = STATE_INPUT_DELAY;
						}
            break;
				case STATE_INPUT_DELAY:
						if(Tim6GetCounterValue() > 10)			/*��ʱ10us*/
						{
								Tim6SetCounterValue(0);
								gAppState = STATE_INPUT_HIGH;
						}
				case STATE_INPUT_HIGH:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==1)	/*�ȴ�Ϊ�ߵ�ƽ*/
							{
									gAppState = STATE_DATA_LOW;
									Tim6SetCounterValue(0);
							}
						}
						else
							gAppState = STATE_WAIT;
            break;
				case STATE_DATA_LOW:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==0)	/*�ȴ�Ϊ�͵�ƽ*/
							{
									gAppState = STATE_DATA_HIGH;
									Tim6SetCounterValue(0);
							}
						}
						else
							gAppState = STATE_WAIT;
            break;
				case STATE_DATA_HIGH:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==1)	/*�ȴ���Ϊ�� �������ݿ�ʼ*/
							{
									Tim6SetCounterValue(0);		/*��Ϊ���Ժ�ʼ��ʱ*/
									gAppState = STATE_DATA_TIME;
							}
						}
						else
							gAppState = STATE_WAIT;
            break;
				case STATE_DATA_TIME:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==0)	/*�ȴ���Ϊ�� �������ݽ���*/
							{
									timeCounter = Tim6GetCounterValue();
								
									/*����counter��ֵ �ж���0����1 ���뵽������*/
									
									if(timeCounter > 40)
									{
											data[counter] = 1;
									}
									if(timeCounter < 40)
									{
											data[counter] = 0;
									}
									
									//data[counter] = timeCounter;
									counter++;	/*�յ���λ����һ*/
									
									/*״̬������ѡ��*/
									if(counter == 40)
									{
											gAppState = STATE_WAIT;	/*�Ѿ��ɼ������*/
											AM2301Data();/*ÿ�βɼ��� ���̴���*/
									}
									else
											gAppState = STATE_DATA_HIGH;								
							}
						}
						else
						{
							gAppState = STATE_WAIT;
						}
						break;
    }
} 

/***********************************************
@������: AM2301_IN_Init ��ʼ�������ߵ���������
@���룺��
@�������
************************************************/
void AM2301_IN_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct;

	/*��ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG ,ENABLE);
	
	/*��ʼ���ܽ�--PG11---��������*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
}

void AM2301_OUT_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct;

	/*��ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG ,ENABLE);
	
	/*��ʼ���ܽ�--PG11--�����������*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
}

/*������ȡ��ת��*/
void AM2301Data(void)
{
	uint8_t i;
	temperatureValue = 0;
	humidityValue = 0;

	for(i=0;i<16;i++)
	{
		crcHumidityValue<<= 1;
		crcHumidityValue+=data[i];
	}
	for(i=16;i<32;i++)
	{
		crcTemperatureValue <<= 1;
		crcTemperatureValue+=data[i];
	}
	for(i=32;i<40;i++)			/*8bitУ��λ*/
	{
		crcValue <<= 1;
		crcValue+=data[i];
	}
	crcHumidityValue = crcHumidityValue + (crcHumidityValue>>8);
	crcTemperatureValue = crcTemperatureValue + (crcTemperatureValue>>8);
	crcCaclulate = crcHumidityValue + crcTemperatureValue;
	
	if(crcCaclulate != crcValue)			/*������� ���� �������κ����ݴ���*/
			return;
	
	for(i=0;i<16;i++)
	{
		humidityValue<<= 1;
		humidityValue+=data[i];
	}
	
	humidity=(float)(humidityValue)/10;
	
	//�¶�ֵ
	for(i=17;i<32;i++)
	{
		temperatureValue <<= 1;
		temperatureValue+=data[i];
	}
	if(data[16])
		temperatureValue=0-temperatureValue;
	temperature=(float)(temperatureValue)/10;
	
	//usart1SendByte(temperature);
	//usart1SendByte(humidity);
}

/**
  * @brief  ��ȡ��ǰ�¶�
  * @param  None
  * @retval None
  */
float * GetTemperature(void)
{
		return &temperature;
}
/**
  * @brief  ��ȡ��ǰʪ��ָ��
  * @param  None
  * @retval None
  */
float * GetHumidity(void)
{
		return &humidity;
}
