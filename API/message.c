#include "message.h"
#include "usart.h"
struct queue messageQueueA;		//˫������л���
struct queue messageQueueB;		//


uint8_t queueSwitch;		//���ڴ������Ϣϵͳ 0��A 1��B

/**
  * @brief  �˺�����ʼ����Ϣ����
  * @param  ���� ��Ϣ���е�ָ�� 
  * @retval ��� ��
  */
void queueInit()
{
	messageQueueA.front = 0;
	messageQueueA.rear = 0;
	messageQueueB.front = 0;
	messageQueueB.rear = 0;
}
/**
  * @brief  �˺���������Ϣ���� �����ǻ᲻�ᱻ�ж�Ӱ��
  * @param  ���� �л���־λ ��Ϣ��ֵ 
  * @retval ��� 0 �ɹ� 1 ��Ϣ��������
  */
uint8_t queueEnqueue(uint8_t queueSwitch, struct message data)		//ok
{
	uint8_t temp;
	switch(queueSwitch)
	{
		case 0:
				if((((messageQueueA.rear) + 1)%QUEUE_SIZE) == messageQueueA.front)		//����
				{
					return 1;
				}
				else
				{
					temp = messageQueueA.rear;
					messageQueueA.message_list[temp] = data;
					messageQueueA.rear = (temp + 1) % QUEUE_SIZE;
					return 0;
				}
				break;
		case 1:
				if((((messageQueueB.rear) + 1)%QUEUE_SIZE) == messageQueueB.front)		//����
				{
					return 1;
				}
				else
				{
					temp = messageQueueB.rear;
					messageQueueB.message_list[temp] = data;
					messageQueueB.rear = (temp + 1) % QUEUE_SIZE;
					return 0;
				}
				break;
	}
}
/**
  * @brief  �˺�����ȡ��Ϣ���� �����ǻ᲻�ᱻ�ж�Ӱ��
  * @param  ���� ��Ϣ���е�ָ�� ��Ϣ��ָ�루���ó���ά���� 
  * @retval ��� 0 �ɹ� 1 ��Ϣ�����ѿ�
  */
uint8_t queueDequeue(uint8_t queueSwitch,struct message * data_pointer)
{
	uint8_t temp;
	switch(queueSwitch)
	{
		case 0:
				if(messageQueueA.front == messageQueueA.rear)							//����
				{
					return 1;
				}
				else
				{
					temp = messageQueueA.front;
					*data_pointer = messageQueueA.message_list[temp];
					messageQueueA.front = (temp + 1)%QUEUE_SIZE;
					return 0;
				}
				break;
		case 1:
				if(messageQueueB.front == messageQueueB.rear)							//����
				{
					return 1;
				}
				else
				{
					temp = messageQueueB.front;
					*data_pointer = messageQueueB.message_list[temp];
					messageQueueB.front = (temp + 1)%QUEUE_SIZE;
					return 0;
				}
				break;
	}
}

/**
  * @brief  �˺���������Ϣ ֻ����������ʹ�ã��жϲ����ã�
			����˫���� ����ʱҪ����������ȡ�Ķ��о����������
  * @param  ���� ��Ϣֵ 
  * @retval ��� ��
  */
void messagePost(struct message data_come)		//������Ϣ
{
	switch(queueSwitch)							
	{
		case 0: queueEnqueue(1,data_come); //����B
				break;
		case 1:	queueEnqueue(0,data_come); //����A
				break;
	}
}
/**
  * @brief  �˺���������Ϣ ֻ���ж���
			����˫���� ����ʱҪ����������ȡ�Ķ��о����������
  * @param  ���� ��Ϣֵ 
  * @retval ��� ��
  */
void messagePostIt(struct message data_come)	//�жϷ�����Ϣ
{
	switch(queueSwitch)							
	{
		case 0: queueEnqueue(1,data_come);
				break;
		case 1:	queueEnqueue(0,data_come);
				break;
	}
}

/**
  * @brief  �˺���������Ϣ���� ��Ҫ��ÿһ����Ϣѭ���� AB����Ϣ�ĸ���
						ͬʱҲ���ʵʱ����ϵͳ�� ÿһ�뱻ִ�е�ʱ�� ����ϵͳ��������
						�˺�������while��������ѭ����
  * @param  ���� ��
  * @retval ��� ��
  */
void sysMonitor()
{
		if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_5) == RESET)
			GPIO_SetBits(GPIOA,GPIO_Pin_5);
		else
			GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		/*ͳ����Ϣ�����е����ж�����Ϣ*/
}
