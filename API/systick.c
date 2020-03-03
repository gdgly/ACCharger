/**
  ******************************************************************************
  * @file    SysTick/TimeBase/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body.
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
#include "systick.h"
#include "can.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup SysTick_TimeBase
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t sysMonitorTimer;	/*ϵͳ���Ӷ�ʱ��*/

static __IO uint32_t TickCount;
static __IO uint32_t tick_10ms;
static __IO uint32_t tick_100ms;
static __IO uint32_t tick_300ms;
static __IO uint32_t tick_500ms;
static __IO uint32_t tick_1000ms;

static __IO uint32_t tick_1ms; //���ʱ����Ĵ���ƫ�Ƶ�ʱ�����
// �������ʮ����Ҫ ��ʼ___ʱ���1__ʱ���2___ʱ���3___��������ʼ��
// ÿһ��ƫ�������Զ���Ϊconst�̶�ֵ Ҳ�����ñ�������ƫ�ƣ�ms����

static uint8_t offset1 = 60;
static uint8_t offset2 = 120;
static uint8_t offset3 = 250;

/*Megmeet ʹ�õ�ʱ�����б��� ͨ�������������޸�ʱ��*/
static uint8_t sequenceValueMegmeet = 0;
/*����Megmeet�жϵ�ms�����ⶨʱ��*/
static uint16_t vMegmeetTimer = 0;	
/*����adc�жϵ�ms�����ⶨʱ��*/
static uint8_t vAdcTimer = 0;	
/*����RC522�жϵ�ms�����ⶨʱ��*/
static uint8_t vRC522Timer = 0;	
/*����W5500�жϵ�ms�����ⶨʱ��*/
static uint8_t vW5500Timer = 0;	
/*��������װ���жϵ�ms�����ⶨʱ��*/
static uint16_t vLockTimer = 0;	
/*����led�жϵ�ms�����ⶨʱ��*/
static uint16_t vLedTimer = 0;	

static uint8_t vUsart2TimerOn = 0;	/*����*/
static uint8_t vUsart2Timer = 0;	/*����usart2�жϵ�ms�����ⶨʱ��*/
static uint8_t vUsart2TimerAlarmValue = 0;

static uint8_t vUsart3TimerOn = 0;	/*����*/
static uint8_t vUsart3Timer = 0;	/*����usart3�жϵ�ms�����ⶨʱ��*/
static uint8_t vUsart3TimerAlarmValue = 0;

static uint8_t vUsart4TimerOn = 0;	/*����*/
static uint8_t vUsart4Timer = 0;	/*����usart4�жϵ�ms�����ⶨʱ��*/
static uint8_t vUsart4TimerAlarmValue = 0;

static uint8_t vCanTimerOn = 0;	/*����*/
static uint8_t vCanTimer = 0;	/*����can protocal�жϵ�ms�����ⶨʱ��*/
static uint8_t vCanTimerAlarmValue = 0;


struct message sysTickMessage;	//�δ��õ���Ϣʵ��

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  init program.
  * @param  None
  * @retval None
  */
int sysTickInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     


  /* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
       call. The NVIC_SetPriority() is defined inside the core_cm3.h file.

    4. To adjust the SysTick time base, use the following formula:
                            
         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF
   */
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
  
  return 0;
}

uint32_t GetTickCount()
{
		return TickCount;
}
void vUsart2TimerSetAlarm(uint8_t alarmValue)
{
		vUsart2Timer = 0;												/*��ʱ�ص�0��*/
		vUsart2TimerAlarmValue = alarmValue;		/*��������ֵ*/
		vUsart2TimerOn = 1;											/*�򿪼�ʱ��*/
}

void vUsart3TimerSetAlarm(uint8_t alarmValue)
{
		vUsart3Timer = 0;												/*��ʱ�ص�0��*/
		vUsart3TimerAlarmValue = alarmValue;		/*��������ֵ*/
		vUsart3TimerOn = 1;											/*�򿪼�ʱ��*/
}

void vUsart4TimerSetAlarm(uint8_t alarmValue)
{
		vUsart4Timer = 0;												/*��ʱ�ص�0��*/
		vUsart4TimerAlarmValue = alarmValue;		/*��������ֵ*/
		vUsart4TimerOn = 1;											/*�򿪼�ʱ��*/
}

void vCanTimerSetAlarm(uint8_t alarmValue)
{
		vCanTimer = 0;												/*��ʱ�ص�0��*/
		vCanTimerAlarmValue = alarmValue;			/*��������ֵ*/
		vCanTimerOn = 1;											/*�򿪼�ʱ��*/
}
void vCanTimerClose()
{
		vCanTimerOn = 0;
}

void vAdcTimerSetValue(uint8_t initValue)
{
		vAdcTimer = initValue;						 /*��ֵ��Ὺʼ�Զ�+1 ��Զ��ͣ*/
}
uint16_t vAdcTimerGetValue()
{
		return vAdcTimer;
}

void vMegmeetTimerSetValue(uint8_t initValue)
{
		vMegmeetTimer = initValue;						 /*��ֵ��Ὺʼ�Զ�+1 ��Զ��ͣ*/
}
uint16_t vMegmeetTimerGetValue()
{
		return vMegmeetTimer;
}

void vRC522TimerSetValue(uint8_t initValue)
{
		vRC522Timer = initValue;						 /*��ֵ��Ὺʼ�Զ�+1 ��Զ��ͣ*/
}
uint16_t vRC522TimerGetValue()
{
		return vRC522Timer;
}

void vW5500TimerSetValue(uint8_t initValue)
{
		vW5500Timer = initValue;						 /*��ֵ��Ὺʼ�Զ�+1 ��Զ��ͣ*/
}
uint16_t vW5500TimerGetValue()
{
		return vW5500Timer;
}

void vLockTimerSetValue(uint16_t initValue)
{
		vLockTimer = initValue;						 /*��ֵ��Ὺʼ�Զ�+1 ��Զ��ͣ*/
}
uint16_t vLockTimerGetValue()
{
		return vLockTimer;
}
void vLedTimerSetValue(uint16_t initValue)
{
		vLedTimer = initValue;						 /*��ֵ��Ὺʼ�Զ�+1 ��Զ��ͣ*/
}
uint16_t vLedTimerGetValue()
{
		return vLedTimer;
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
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
		/*ÿ 1ms Ҫִ�еĲ��� ע��˲�������Ƶ�� ����������г���nms����ʱ ��ô
		��Ϣ���о��п��ܻ��ۺܶ���Ϣ Ӱ�쵽һЩʵʱ���� ���ϵͳ���ȶ�*/
		sysTickMessage.destinationAddress = 1;	
		sysTickMessage.cmd = 0;
		messagePostIt(sysTickMessage);
		
		tick_1ms++;
	
		/*��������ͨ��ʱ��*/
		vMegmeetTimer++;				/*Megmeet����ר�õĶ�ʱ��*/
		vAdcTimer++;
		vRC522Timer++;
		vW5500Timer++;
		vLockTimer++;
		vLedTimer++;

		/*���������Ӷ�ʱ��*/
		if(vUsart2TimerOn == 1)
		{
				vUsart2Timer++;	
				if(vUsart2Timer > vUsart2TimerAlarmValue)	/*˵����ʱ��*/
				{
						/*��������� ˵����ʱ��������� ��ô��Ϊ�����һ֡*/
						/*���� ready = 1 ������Բ������ַ�ʽ��ready��һ*/
						/*��һ ����һ����Ϣ��״̬�� ���������ճ�ʱ һ֡�Ѿ��յ�*/		
						/*�ڶ� ֱ����readyΪ1 ������������ ����������������*/
						Usart2SetReady();

						vUsart2TimerOn = 0;	/*ʹ����� �ر�����*/
				}
		}
		if(vUsart3TimerOn == 1)
		{
				vUsart3Timer++;	
				if(vUsart3Timer > vUsart3TimerAlarmValue)	/*˵����ʱ��*/
				{
						/*��������� ˵����ʱ��������� ��ô��Ϊ�����һ֡*/
						/*���� ready = 1 ������Բ������ַ�ʽ��ready��һ*/
						/*��һ ����һ����Ϣ��״̬�� ���������ճ�ʱ һ֡�Ѿ��յ�*/		
						/*�ڶ� ֱ����readyΪ1 ������������ ����������������*/
						Usart3SetReady();
						//usart1SendByte(0xcc);
						vUsart3TimerOn = 0;	/*ʹ����� �ر�����*/
				}
		}
		if(vUsart4TimerOn == 1)
		{
				vUsart4Timer++;	
				if(vUsart4Timer > vUsart4TimerAlarmValue)	/*˵����ʱ��*/
				{
						/*��������� ˵����ʱ��������� ��ô��Ϊ�����һ֡*/
						/*���� ready = 1 ������Բ������ַ�ʽ��ready��һ*/
						/*��һ ����һ����Ϣ��״̬�� ���������ճ�ʱ һ֡�Ѿ��յ�*/		
						/*�ڶ� ֱ����readyΪ1 ������������ ����������������*/
						Usart4SetReady();
						//usart1SendByte(0xcc);
						vUsart4TimerOn = 0;	/*ʹ����� �ر�����*/
				}
		}
		if(vCanTimerOn == 1)
		{
				vCanTimer++;	
				if(vCanTimer > vCanTimerAlarmValue)	/*˵����ʱ��*/
				{
						/*��������� ˵����ʱ��������� ��ô��Ϊ�����ж���*/
						
						/*��һ ����һ����Ϣ��״̬�� ���������ճ�ʱ �����ж�*/		
						/*�ڶ� ֱ����readyΪ1 ������������ �����ж�*/
						LinkClose();
						
						vCanTimerOn = 0;	/*ʹ����� �ر�����*/
				}
		}
		
		if (tick_1ms == offset1)
		{

		}
		if (tick_1ms == offset2)
		{
				
		}
		if (tick_1ms == offset3)
		{
				
				tick_1ms = 0;
		}
		
		
    if (++tick_10ms >= 10)   // 10ms��ʱ
    {
        tick_10ms = 0;
            
        // ÿ 10ms Ҫִ�еĲ���
        // ...
    }
		if (++tick_100ms >= 100)   // 100ms��ʱ
    {
        tick_100ms = 0;
        sequenceValueMegmeet++;
        // ÿ 100ms Ҫִ�еĲ���
        // ...
				sysTickMessage.destinationAddress = 6;	/*ÿ100ms����һ�� ��ѹ�������������*/
				sysTickMessage.cmd = 1;								/*cmd = 0������ 1������*/
				messagePostIt(sysTickMessage);

				/*����sequenceValueMegmeet��ִ������*/
				if(sequenceValueMegmeet == 2)
				{
						sysTickMessage.destinationAddress = 7;	/*ÿ100ms����һ�� */
						sysTickMessage.cmd = 0;								
						messagePostIt(sysTickMessage);
				}
				if(sequenceValueMegmeet == 3)
				{
						sysTickMessage.destinationAddress = 7;	/*ÿ100ms����һ�� */
						sysTickMessage.cmd = 1;								
						messagePostIt(sysTickMessage);
				}
				if(sequenceValueMegmeet == 4)
				{
						sysTickMessage.destinationAddress = 7;	/*ÿ500ms����һ�� */
						sysTickMessage.cmd = 0;								
						messagePostIt(sysTickMessage);
				}
				if(sequenceValueMegmeet == 6)
				{
						sysTickMessage.destinationAddress = 7;	/*ÿ500ms����һ�� */
						sysTickMessage.cmd = 2;								
						messagePostIt(sysTickMessage);
						
						sequenceValueMegmeet = 0;
				}
			
    }
		if (++tick_300ms >= 300)   // 300ms��ʱ
    {
        tick_300ms = 0;
            
        // ÿ 300ms Ҫִ�еĲ���
        // ...
				sysTickMessage.destinationAddress = 2;
				sysTickMessage.cmd = 0;
				messagePostIt(sysTickMessage);

				sysTickMessage.destinationAddress = 8;	/*sysMonitor*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);
    }
		if (++tick_500ms >= 500)   // 500ms��ʱ
    {
        tick_500ms = 0;
            
        // ÿ 500ms Ҫִ�еĲ���
        // ...
				sysTickMessage.destinationAddress = 3;
				sysTickMessage.cmd = 0;
				messagePostIt(sysTickMessage);
				
				sysTickMessage.destinationAddress = 10;	/*RFID*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);	
			
				sysTickMessage.destinationAddress = 13;	/*�������ؼ��*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);	
    }
		if (++tick_1000ms >= 1000)   // 1000ms��ʱ
    {
        tick_1000ms = 0;
			
        TickCount++;  // ʱ��δ�ֵ  (1s)  
        // ÿ 1s Ҫִ�еĲ���
        // ...
			
				sysTickMessage.destinationAddress = 6;	/*ÿ1s����һ�� ��ѹ�������������*/
				sysTickMessage.cmd = 0;								/*cmd = 0������ 1������*/
				messagePostIt(sysTickMessage);
			
				sysTickMessage.destinationAddress = 12;	/*ÿ1s����һ�� rtc*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);

    }
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
