/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MESSAGE_H
#define __MESSAGE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/
#define QUEUE_SIZE  50	//��Ϣ���еĴ�С
/* Exported types ------------------------------------------------------------*/
struct message		//���ݵ���Ϣͨ�ýṹ
{
	uint8_t sourceAddress;			//Դģ���ַ
	uint8_t destinationAddress;    //Ŀ��ģ���ַ
	uint8_t cmd;					//����
	uint8_t parameter_1;			//4������ �������¶� ������mac
	uint8_t parameter_2;
	uint8_t parameter_3;
	uint8_t parameter_4;
};
struct queue		//������Ϣͨ�ýṹ��˳��洢ѭ������
{
	struct message message_list[QUEUE_SIZE];
	uint8_t front;
	uint8_t rear;
	uint8_t size;
};
/* Exported constants --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

void queueInit(void);
uint8_t queueEnqueue(uint8_t queueSwitch ,struct message data);
uint8_t queueDequeue(uint8_t queueSwitch ,struct message * data_pointer);
extern void messagePost(struct message data_come);	
extern void messagePostIt(struct message data_come);
void sysMonitor(void);
#endif /* __SYSTICK_H */
