#include "table.h"
#include "string.h"
#include "pgn.h"

#define STATE_SIZE            30		//�ܹ��ж��ٸ�state
#define STATE_TRANSITION_SIZE 23  	//Ǩ����Ŀ�ĸ���

conditionQueue cantriggerQueue;
struct message transitionMessage;	//Ǩ���õ���Ϣʵ��

/****************************************************
��������stateMachineSwitcher
������״̬���Ŀ��� 0���ر� 1������
****************************************************/
uint32_t stateMachineSwitcher;

/****************************************************
��������timeBase 
��������1msΪ��λ��ʱ���� ����Լ��㵽49.71026��/���
������ÿһ��״̬��ʱ����ṩһ����׼����BMS��ʼʱ����
��BMS����ʱ��0��Ϊ״̬������ʱ������
****************************************************/
uint32_t timeBase;	

/****************************************************
��������activeState 
���������״̬
****************************************************/
uint8_t activeState;	

/****************************************************
��������stateChanged 
������1��״̬�Ѿ��ı� 0��״̬û�иı�
��һ��������Ǩ�Ʊ��н���
���㶯������ʱ����н��У�һ���������㣩
****************************************************/
uint8_t stateChanged;	

/****************************************************
��������stateTransitionItem  ״̬Ǩ����
������Դ״̬+��������+Ŀ��״̬+����
��������ֻ������Ϣ���Լ�������ʱ����ϢҲ�ɣ�
����ֻ������Ϣ
****************************************************/
typedef struct 
{
	uint8_t sourceState;		//Դ״̬
	uint8_t triggerCondition;	//��������
	uint8_t targetState;		//Ŀ��״̬
	uint8_t action;				//����
}stateTransitionItem;

/****************************************************
��������stateTransitionTable  ״̬Ǩ�Ʊ�
��������n��stateTransitionItem��ɣ����������е�״̬
Ǩ�ƣ������߼��ϵĺ�ʱ���ϵ� 
----<ԭ״̬���������룬Ŀ��״̬������>-----
****************************************************/
stateTransitionItem stateTransitionTable[STATE_TRANSITION_SIZE] = 
{
		{0, 	 0, 1,    0},
		{1, 0xe1, 2, 0xb3},
		{1, 0xe2, 2, 0xb3},
		{1, 0xe3, 2, 0xb3},
		{2, 0xe2, 3, 0xb3},
		{3, 0xe3, 2, 0xb3},
		{2, 0xe4, 1, 0xb3},
		{2, 	 2, 5, 0xb1},
		{3,    3, 6, 0xb1},
		{6, 0xe5, 3, 0xb3},
		{6,    6, 6, 0xb2},
		{6, 0xe6, 5, 0xb1},
		{6, 0xea, 7, 0xb3 },
		{5, 0xe7, 4, 0xb1},
		{4, 0xe8, 1, 0xb3},
		{4, 0xe9, 5, 0xb1},
		{5, 	 5, 2, 0xb3},
		/*����ʱ���*/
		{1, 0xf1, 7, 0},
		{2, 0xf2, 7, 0},
		{3, 0xf3, 7, 0},
		{4, 0xf4, 7, 0},
		{5, 0xf5, 7, 0},
		{2, 0xf6, 7, 0},
};
#if 0
stateTransitionItem stateTransitionTable[STATE_TRANSITION_SIZE] = 
{
	{0, 0, 1, 0},							/*0״̬��ʱ�ͽ���1״̬ ִ�ж���Ϊ0���ޣ�*/
	{1, 1, 2, 0xc1},					/*1״̬��ʱ10ms�ͽ���2״̬ ִ�з���CHM*/
	{2, 2, 1, 0},							/*2״̬��ʱ250ms�ͽ���1״̬*/
	{2, 0xb1, 4, 0},					/*2״̬���յ�BHM �ͽ���4״̬*/
	{2, 0xe1, 3, 0},					/*2״̬����ʱ5s�ͽ���3״̬*/
	{3, 3, 4, 0},							/*3״̬��ʱ10ms�ͽ���4״̬ �൱�ں�����*/
	
	{4, 4, 5, 0xc2},					/*4״̬��ʱ10ms�ͽ���5״̬ ִ�з���CRM 0x00*/
	{5, 0xb2, 6, 0},					/*5״̬���յ�BRM �ͽ���4״̬*/
	{5, 5, 4, 0},							/*5״̬��ʱ250ms�ͽ���4״̬*/
	{5, 0xe2, 8, 0},					/*5״̬����ʱ5S�ͽ���8״̬*/
	{6, 6, 7, 0xc2},					/*4״̬��ʱ10ms�ͽ���5״̬ ִ�з���CRM 0xAA*/
	{5, 0xe2, 8, 0},
//	{8, 8, ERROR, ERROR_ACTION},/*8״̬��ʱ10ms�ͽ�����ʧ�ܸ澯״̬ ִ�и澯����*/
	
	{7, 7, 6, 0},							/*7״̬��ʱ250ms�ͽ���6״̬ ���·��ͱ���*/
	{7, 0xe3, 10, 0},					/*7״̬����ʱ5s�ͽ���10״̬*/
	{7, 0xb3, 8, 0},					/*7״̬�յ�BCP �ͽ���8״̬*/
	{8, 8, 9, 0xca},					/*8״̬��ʱ10ms �ͽ���9״̬ ͬʱִ�з���CTS��CML*/
	{9, 0xe4, 11, 0},					/*9״̬����ʱ5s �ͽ���11״̬*/
	{9, 9, 8, 0},							/*9״̬��ʱ250ms �ͽ���8״̬*/
	{9, 0xb4, 12, 0},					/*9״̬�յ�BRO  �ͽ���12״̬*/
	{12, 12, 12, 0xc5},				/*12״̬��ʱ250ms  �ͽ���12״̬*/
	{12, 0xe5, 14, 0},				/*12״̬����ʱ5s  �ͽ���14״̬*/
	{12, 0xba, 13, 0},				/*12״̬�ж�ready  �ͽ���13״̬*/
	{13, 13, 15, 0xc5},				/*13״̬��ʱ10ms �ͽ���15״̬ ִ�з���CRO*/
	
	{15, 0xeb, 13, 0},				/*15״̬û���յ�BCS BCL �ͽ���13״̬*/
	{15, 0xec, 16, 0},				/*15״̬�յ�BCS BCL �ͽ���16״̬*/
	{15, 0xe6, 17, 0},				/*15״̬����ʱ���ͽ���17״̬*/
	{17, 17, 17, 0},					/*17״̬��ʱ���ͽ���17״̬*/
	
	{16, 16, 18, 0xc6},				/*16״̬��ʱ10ms  �ͽ���18״̬ ִ�з���CCS */
	{18, 0xEC, 19, 0},				/*18״̬�ɹ����վͽ���19״̬ */
	{18, 0xEB, 21, 0},				/*18״̬���ճ�ʱ�ͽ���21״̬ */

	
	{19, 0xb8, 20, 0},					/*19״̬�յ�BST �ͽ���20״̬ */
//	{19, MODULECLOSE, 20, 0}, /*19״ֹ̬ͣ״̬ �ͽ���20״̬ */
	{19, 19, 16, 0},					/*19״̬��ʱ250ms �ͽ���16״̬ */
	{20, 20, 22, 0xc7},				/*20״̬��ʱ10ms �ͽ���22״̬ */
	{22, 22, 20, 0},				  /*22״̬��ʱ250ms �ͽ���20״̬ */
	{22, 0xe9, 24, 0},				/*20״̬����ʱ5s �ͽ���24״̬ */
	
	{23, 0xb9, 24, 0},				/*23״̬�յ�BSD �ͽ���24״̬ */
	{23, 23, 20, 0},					/*23״̬��ʱ250ms �ͽ���20״̬ */
	{23, 0xea, 25, 0},				/*20״̬����ʱ5s �ͽ���25״̬ */
	{24, 24, 26, 0xc8},				/*24״̬��ʱ10ms �ͽ���26״̬ ִ�з���CSD*/
	
};
#endif
/****************************************************
��������stateTimeItem  ״̬Ǩ����
��������¼��״̬���п��ܵ�ʱ��� ͨ�����������ж�
״̬�ǲ��ǳ�ʱ�������Ĵ����ǲ���̫�࣬�涨ʱ������û��
���ĳһ�����ȵȣ���Ǩ�Ʊ��ʱ�����ԡ�
****************************************************/
typedef struct 
{
	uint32_t firstTime;		//״̬�ʼʱ��
	uint32_t latestTime;	//״̬���һ��ʱ��
	uint32_t duration;		//״̬���һ�γ���ʱ��
	uint32_t times;			//״̬�ܹ�����
}stateTimeItem;

/****************************************************
��������stateTimeTable  ״̬ʱ���
��������n��stateTimeItem��ɣ����������е�״̬
��ʱ��
****************************************************/
stateTimeItem stateTimeTable[STATE_SIZE];

/****************************************************
��������conditionsQueue  ��������
��������ĳ��state��������£����в�����������֮����
��һ�����У�����Ϊ�����Ĳ���Ƶ���п��ܴ���Ǩ�Ƹ��µ�Ƶ��
Ϊ�˷�ֹ���������������Բ����˶���
�ڷ����˳ɹ���Ǩ��֮��Ҫ�Ѷ�����ʣ����������㣬��ֹ
Ӱ����һ��״̬�������߼�����
****************************************************/
conditionQueue triggerQueue;

/****************************************************
��������tableInit
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ʼ��״̬���ĸ���������ص�
״̬���ʼ��״̬ 
****************************************************/
void tableInit()
{
	uint8_t counter;
	stateMachineSwitcher = 0; //�ر�״̬��
	timeBase = 0;			  //ʱ��������
	activeState = 0;		  //��ʼ״̬
	for(counter = 0; counter < STATE_SIZE; counter++)
	{
		stateTimeTable[counter].firstTime = 0;
		stateTimeTable[counter].latestTime = 0;
		stateTimeTable[counter].duration = 0;
		stateTimeTable[counter].times = 0;
	}
	triggerQueue.size = 0;	  //�����������
	
	/*״̬����Ҫ���Ƶ������Ͷ���ҲҪ�����ʼ״̬*/
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	GPIO_ResetBits(GPIOE, GPIO_Pin_4);
}
/****************************************************
��������messageTimeItem ����ʱ����Ʊ�
������ͨ���Ա��ļ�¼ʱ�䣬��ʵ�ֶԲ����߼��ı�Ҳ��
��˵�����ҵĺ������ͬʱ������ͨ�������Ժܷ�����ͬ
ʱ�� start�����ļ�������ͬʱ���м��飬һ��start��Ч��
��Ŀ�У�ȫ��times��Ϊ1�������1������ô����ȫ�����յ���
Ȼ���ж��Ƿ�ʱ�������ͬʱ����ⳬʱ��������������
ô˵��֮ǰ��times������Ҫ�󣬲Ż��⵽��ʱ�ġ�
ֻ���ڱ�ﱨ�Ľ���֮����߼���ʱ�򣬲Ż���ô˱�
���ȫ�����ô˱���ô���ĵĳ�ʱ�жϾͿ�����ȡ��������
����״̬�������жϣ��ṹ��������Ҳ�ǲ����Ժ�����
****************************************************/
typedef struct 
{
	uint8_t  start;						//��ʼ��־ 0��ֹͣ����0���ݣ� 1����ʼ
	uint32_t accumulatedTime;	//�ۻ�ʱ��
	uint32_t overFlowTime;		//���ʱ��
	uint32_t times;						//�յ�����
}messageTimeItem;

messageTimeItem messageTimeTable[3] = 
{
	{0 , 0, 50, 0},			/*BCL���ĳ�ʱ����*/
	{0 , 0, 250, 0},		/*BCS���ĳ�ʱ����*/
	{0 , 0, 250, 0},		/*BSM���ĳ�ʱ����*/
};

/****************************************************
��������stateMachineOpen
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ʼ��״̬���ĸ���������ص�
״̬���ʼ��״̬ 
****************************************************/
void stateMachineOpen()
{
	tableInit();
	stateMachineSwitcher = 1; //��״̬��
}

/****************************************************
��������stateMachineClose
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ʼ��״̬���ĸ���������ص�
״̬���ʼ��״̬ 
****************************************************/
void stateMachineClose()
{
	stateMachineSwitcher = 0; //�ر�״̬��
	tableInit();
}

/****************************************************
��������stateTransitionUpdate��10ms��Ϣ������
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У�ÿ10msִ��һ��Ǩ��
�Ȼ��ڻ״̬ Ȼ���ٽ������ ��Ǩ�Ʊ��в��ң�û�ҵ�������
����ҵ����״̬�ȸ��� Ȼ����ִ�ж�������Ϣ�� 
Ǩ�Ʊ�ֻǿ���߼�:
1.����û�ж�ʱ��Ͷ��������涨��
2.�����������Ǩ�Ƶ�һ��״̬��һ��״̬�����ж��������ͼ�ṹ��
****************************************************/
void stateTransitionUpdate()
{
	uint8_t tCounter = 0;
	uint8_t cCounter = 0;
	
	if(stateMachineSwitcher == 0)	//˵��״̬�����ڹر�״̬
		return;
		
	//���Ȳ����������� ����� ���� ���û�� return
	if(triggerQueue.size == 0)
		return; 
		
	//�Ա����б��� Ѱ����activeState��ͬ����Ŀ
	//Ҳ����˵ ����activeState�Ĵ�����������ʲô
	for(tCounter = 0; tCounter < STATE_TRANSITION_SIZE; tCounter++)
	{
		if(stateTransitionTable[tCounter].sourceState == activeState)
		{	//�ҵ�Դ״̬==activeState��Ǩ����
			for(cCounter = 0 ; cCounter < triggerQueue.size; cCounter++)
			{	
				//�����������������һһ�Ա�
				if(stateTransitionTable[tCounter].triggerCondition == \
					triggerQueue.queue[cCounter])
				{
					//ִ�д�����Ķ���....
					usart1SendByte(0xd1);
					//���ȸ��Ļ״̬
					activeState = stateTransitionTable[tCounter].targetState;
					//��������������
					triggerQueue.size = 0;
					//״̬�ı��־λ
					stateChanged = 1;
					//ִ��ĳЩ����....
					if(stateTransitionTable[tCounter].action != 0) /*ȷʵ����Ч�Ķ�������*/
					{
							transitionMessage.destinationAddress = 4;
							transitionMessage.cmd = 2;
							transitionMessage.parameter_1 = stateTransitionTable[tCounter].action;
							messagePost(transitionMessage);
					}
				}
				else	/*Ǩ������û�б�����*/
				{
					//֪ͨϵͳ���õ��������������ڵ���
				}
				if(stateChanged == 1)
					break;
			}
		}
		if(stateChanged == 1)
				break;
	}
	
	/*��������֮��û�����activeState�����������㣬Ҳ����˵
	������Щ�ڶ�������������������activeState�ģ���ô��Щ
	��������״̬����˵�����壬Ӧ�ñ�����*/
	/*��û�ж�Ҫ����*/
	triggerQueue.size = 0;
	
}

/****************************************************
��������stateTimeUpdate��1ms��Ϣ������
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У�ÿ1ms����һ��timeBase
�жϻ��״̬�Ƿ�ı���stateChanged����
���������µ�״̬����ô������״̬��duration
���������µ�״̬����ô����times++
ֻҪfirstTimeΪ0����ô�ͼ�¼ΪtimeBase
���ڻ��״̬��duration++����¼��
���ڻ��״̬latestTime��¼ΪtimeBase
ʱ�����Ǩ�Ʊ��ʱ�����ԣ�
1.������ÿһ��״̬����ϸʱ������
****************************************************/
void stateTimeUpdate()
{
	uint8_t counter;			/*��ͨ����*/
	
	if(stateMachineSwitcher == 0)	//˵��״̬�����ڹر�״̬
		return;

	timeBase++;					//ÿ1ms��һ
	if(stateChanged)    //״̬������Ǩ��,״̬�ı���
	{
		stateTimeTable[activeState].duration = 0;
		stateTimeTable[activeState].times++;

		//�����״̬��firstTimeΪ0 
		if(stateTimeTable[activeState].firstTime == 0)
		{
			//����ʱ���
			stateTimeTable[activeState].firstTime = timeBase;
		}
		stateChanged = 0;	//����־λ���� ����ʱ�������˵�Ѿ�����
	}
	
	stateTimeTable[activeState].duration++; //״̬��������
	stateTimeTable[activeState].latestTime = timeBase; //��������ʱ��
	
	/*������״̬���ʱ����ƣ������Ǳ��ı��ʱ�����*/
	/*��timeUpdate������ ���ֻ��������¼ʱ�䣬��������ʱ����ж�*/
	for(counter = 0 ; counter < 3; counter ++)
	{
			if(messageTimeTable[counter].start == 1)
			{
					messageTimeTable[counter].accumulatedTime++;
			}
			else
			{
					messageTimeTable[counter].accumulatedTime = 0;	/*ÿ1ms������*/
					messageTimeTable[counter].times = 0;
			}
	}
	
}
/****************************************************
��������timeOutTable 
������ͣ����ÿһ��״̬��timeOutʱ�� �˴�����STATE_SIZE��ʼ��ֵ
			��������ĳ���
****************************************************/
/*const uint32_t timeOutTable[STATE_SIZE] = 
{
 10,10,250,10,10,250,10,10,10,10,
 10,10,250,10,10,250,10,10,10,10,
 10,10,250,10,10,250,10,10,10,10
};*/
const uint32_t timeOutTable[STATE_SIZE] = 
{
 1000,1000,1000,1000,1000,1000,1000,10,10,10,
 10,10,250,10,10,250,10,10,10,10,
 10,10,250,10,10,250,10,10,10,10
};
/****************************************************
��������conditionTable 
����������PE3 PE4 ����1 ����4��ֵ ӳ�䵽 ��������
****************************************************/
const uint8_t conditionTable[14][5] = 
{//PE3 PE4 1  4 code
	{0, 0, 1,  1 ,  0xe1},
	{0, 0, 1,  3 ,  0xe2},
	{0, 0, 1,  0 ,  0},
	{0, 0, 1,  2 ,  0xe3},
	{0, 0, 0,  1 ,  0xe4},
	{0, 0, 0,  2 ,  0xe4},
	{0, 0, 0,  3 ,  0xe4},
	{1, 1, 1,  5 ,  0xe6},
	{1, 0, 0,  4 ,  0xe7},
	{1, 0, 0,  5 ,  0xe7},
	{1, 0, 0,  6 ,  0xe7},
	{1, 0, 0,  0 ,  0},
	{1, 0, 1,  5 ,  0xe9},
	{1, 1, 0,  4 ,  0xea}
};
/****************************************************
��������AdcValueToState ����PE3��ֵ��ADCƽ��ֵ��ȷ��CP CCĿǰ��״̬

�βΣ���
����ֵ����
�������ܣ��ڴ˺����� ����PWM�͵�ƽ���ױ�׼ȷ��CP��״̬
��ƽ: 12V   1    9V   2     6V    3    ����: 0
PWM:  12V   4    9V   5     6V    6
****************************************************/
uint8_t AdcValueToCPstate()
{
		uint8_t PE3;
		uint16_t adcAVGValue;
		//�����ж�PWM���ǵ�ƽ
		PE3 = GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_3);
		adcAVGValue = adc3Value();
		//���ݸ��Ա�׼�ж���12V 9V 6V ��������
		if(PE3 == 0)	
		{
				/*Ŀǰ���ڵ�ƽ���״̬*/
				if(adcAVGValue>3000 && adcAVGValue<3430)
						return 1;
				else if(adcAVGValue>2500 && adcAVGValue<2900)
						return 2;
				else if(adcAVGValue>1500 && adcAVGValue<2000)
						return 3;
				else
						return 0;
		}
		else
		{
				/*Ŀǰ����PWM���״̬*/
				if(adcAVGValue>2700 && adcAVGValue<3000)
						return 4;
				else if(adcAVGValue>2350 && adcAVGValue<2500)
						return 5;
				else if(adcAVGValue>1400 && adcAVGValue<1700)
						return 6;
				else
						return 0;
		}
}
uint8_t AdcValueToCCstate()
{
		uint16_t adcAVGValue;
		//�����ж�PWM���ǵ�ƽ	
		adcAVGValue = adc3Value();
	
		if(adcAVGValue<2900 && adcAVGValue >600)
				return 1;
		else
				return 0;
		
		
		//���ݸ��Ա�׼�ж���12V 9V 6V ��������
		if(adcAVGValue>1800 && adcAVGValue<2500)
			return 0;
		else if(adcAVGValue>0 && adcAVGValue< 200)
			return 1;
		else
			return 3;
}
/****************************************************
��������conditionUpdate �����ռ���10ms��Ϣ������
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У�ÿ1ms����һ������ ���Ƕ���
ʱ��Ҳ�ǿ��Բ���Ǩ�������ģ��ڴ˺�����ҲҪ��ʱ�俼��
��˿��ܲ����������� ���߸���
�ܽ����2�� :�߼�Ǩ�ƺ�ʱ��Ǩ�ƣ�
****************************************************/
void conditionUpdate()
{
	uint8_t cCounter;		//�����ñ���
	uint8_t tCounter;		//ʱ���ñ���
	uint8_t startFlag;	//��Ч��־
	uint8_t resultFlag;
	uint8_t timeOutFlag;
	uint8_t PE3,PE4,M1,M4;
	
	if(stateMachineSwitcher == 0)	//˵��״̬�����ڹر�״̬
		return;
	
	/*��ȡʵʱ����*/
	PE3 = GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_3);
	PE4 = GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_4);
	M1 = AdcValueToCCstate();
	M4 = AdcValueToCPstate();
	//ɨ���߼�����
	for(cCounter = 0; cCounter < 14; cCounter++)
	{
		if(PE3 == conditionTable[cCounter][0] && \
		   PE4 == conditionTable[cCounter][1] && \
		    M1 == conditionTable[cCounter][2] && \
		    M4 == conditionTable[cCounter][3] )
		{
			//����������д��
			triggerQueue.queue[triggerQueue.size] = conditionTable[cCounter][4];
			triggerQueue.size++;
			break; //�ҵ�һ���������˳� ��Ϊ�������������
		}
	}
	
	/*ɨ��ʱ�����������activeState�ģ�*/
	/*ʱ����������ʹduration����ʱ��̫���ˣ�Ҳ�����ǲ�����״̬֮��
	��ʱ��̫���ˣ���������firsttime��ĳ��һ״̬������ʱ�䣬ʹ����
	���Ƚ������˳���ʱ��duration������������������ݽṹ��ʾ��
	������Ƚϸ��ӣ���������Գ���Ĳ��죬��������ʵ�֡�duration��
	���ñ���ʾ��*/
	
	/*���ȼ��durition�����*/
	if(stateTimeTable[activeState].duration >= timeOutTable[activeState])
	{
			//������ʱ����������...
			ConditionInsert(&triggerQueue, activeState); /*activeState=��������*/
	}
	
	//�ټ��������ֿ�״̬��ʱ���������...

	if(activeState == 1)
	{
			if((stateTimeTable[1].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF1); /*���������0xe1��ʼ*/
			}
	}
	if(activeState == 2)
	{
			if((stateTimeTable[2].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF2); 
			}
	}
	if(activeState == 3)
	{
			if((stateTimeTable[3].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF3); 
			}
	}
	if(activeState == 4)
	{
			if((stateTimeTable[4].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF4); 
			}
	}
	if(activeState == 5)
	{
			if((stateTimeTable[5].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF5); 
			}
	}
	
	/*ÿ��״̬�Ĵ���������*/
	if(activeState == 2)
	{
			if(stateTimeTable[2].times > 10)
			{
					ConditionInsert(&triggerQueue, 0xF6); 
			}
	}
	//..

#if 0	
	/*�����ռ�can���ձ��ĵ����*/
	while(cantriggerQueue.size--)
	{
			ConditionInsert(&triggerQueue, cantriggerQueue.queue[cantriggerQueue.size-1]);
	}
	
	/*�����ռ�����megmeetģ�����Ϣ*/
	//...
	
	/*��������Բ��ֱ���ʱ����������*/
	/*����˼�����ȼ����Чλ��times�����timesû������Ҫ����ô�ټ�鳬ʱ*/
	/*���ó�ʼ״̬*/
	startFlag = 0;
	resultFlag = 1;			 /*�ٶ��ɹ�������*/
	timeOutFlag = 0;
	/*�ȼ����û����Чλ*/
	for(tCounter = 0; tCounter < 3; tCounter++)
	{
			if(messageTimeTable[tCounter].start == 1)
					startFlag = 1;
	}
	if(startFlag == 1)	/*�����⵽��Ҫ���ı���*/
	{
			for(tCounter = 0; tCounter < 3; tCounter++)
			{
					if((messageTimeTable[tCounter].start == 1)&&(messageTimeTable[tCounter].times == 0))
							resultFlag = 0;		/*���˽���ʧ��*/							
					
			}
			if(resultFlag == 0)	/*û�гɹ����� �ͼ�鳬ʱ�����*/
			{
					for(tCounter = 0; tCounter < 3; tCounter++)
					{
							if((messageTimeTable[tCounter].start == 1)&&\
								(messageTimeTable[tCounter].accumulatedTime >\
									messageTimeTable[tCounter].overFlowTime))
							{
									timeOutFlag = 1;	/*�����Ѿ���ʱ�� ���������жϾ�û��������*/
									
									/*֪ͨ״̬�� ���ͳ�ʱ�ź� ��ʽ�ǽ��봥������*/
									ConditionInsert(&triggerQueue, 0xEB);
								
									/*�رմ˴μ�ʱ*/
									memset(messageTimeTable,0,sizeof(messageTimeTable));
							}
					}
			}
			else		/*�ɹ�������*/
			{
					/*֪ͨ״̬�� ���ͳɹ������ź�*/
					ConditionInsert(&triggerQueue, 0xEC);
			}
	}  /*if(startFlag == 1)*/
	else
	{
			/*û��Ҫ���ı��� ��ʲôҲ����*/
	}
#endif
}
/****************************************************
��������conditionInsert
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ָ�����������в���һ����������
****************************************************/
void ConditionInsert(conditionQueue * pC ,uint8_t conditionCode)
{
		pC->queue[pC->size] = conditionCode; 
		pC->size++;
}
/****************************************************
��������messageRecvUpdate ���ⲿ����API ���Ǹ�����һ�ֱ����ܵ���
				����0��ʱ������һ�´���
�βΣ�counter ָ������һ�ֱ��� BCL BCS ��������
����ֵ����
�������ܣ��ڴ˺����� �����յ�����ر��ĺ� ����messageTimeTable
�Ĳ�����һ�����ⲿ���ã�
�����Ժ� ����times�϶�Ҫ��һ
Ȼ���ʱ�����㣬���¿�ʼ��ʱ
****************************************************/
void messageRecvUpdate(uint8_t counter)
{
		messageTimeTable[counter].accumulatedTime = 0;
		messageTimeTable[counter].times++;
}

/****************************************************
��������messageTimeTableSet ���ⲿ����API ����˵��һ�ֱ��Ľ���
���ϵ�����㡣����ʼ�����ݡ���������ϵ���ڳ�ʱ���Զ����еġ�
�βΣ���
����ֵ����
�������ܣ��ڴ˺����� �����յ�����ر��ĺ� ����messageTimeTable
�Ĳ�����һ�����ⲿ���ã�
�����Ժ� ����times�϶�Ҫ��һ
Ȼ���ʱ�����㣬���¿�ʼ��ʱ
****************************************************/
void messageTimeTableSet(uint8_t counter)
{
		messageTimeTable[counter].start = 1;
		/*˳��������������0һ��*/
		messageTimeTable[counter].accumulatedTime = 0;
		messageTimeTable[counter].times = 0;
}

/****************************************************
��������ActionExecutor��֮���Բ���actionUpdate ����Ϊ����ʱ����Ϣ���� ������ִ�еģ�
�βΣ���
����ֵ����
�������ܣ��ڴ˺����� ��Ҫ����conditionUpdate�����ĸ���
����ִ����Ϣ��������Ϣ��Ҫ�Ǳ��룬���Ա���һ��Ҫ�Ͷ���
ӳ�����������ڶ���ִ�е�ʱ���Ǽ��̵ģ���˲����ʵʱ��
����Ӱ�졣��ִ�л�����Ϊ״̬����ִ�з��� ʮ����Ҫ
��Ҫ�Ķ����У�1 can֡����
							2 megmeet������ѹ����     ���ͨ������Ѿ����
							3 �̵�������
							4 ��������
****************************************************/
void ActionExecutor(struct message actionMessage)		/*��������ǧ����� cmd���Ǳ���*/
{
		/*����PGN.c�еķ��ͺ��� ����*/
		//ActionHandle(actionMessage); /*canTaskֻʶ���Լ���ʶ�ı��� ���ڲ���ʶ�ı������*/
		switch(actionMessage.parameter_1)
		{
			case 0xb1:
				GPIO_SetBits(GPIOE,GPIO_Pin_3);
				GPIO_ResetBits(GPIOE,GPIO_Pin_4);
				break;
			case 0xb2:
				GPIO_SetBits(GPIOE,GPIO_Pin_3);
				GPIO_SetBits(GPIOE,GPIO_Pin_4);
				break;
			case 0xb3:
				GPIO_ResetBits(GPIOE,GPIO_Pin_3);
				GPIO_ResetBits(GPIOE,GPIO_Pin_4);
				break;
			case 0xb4:
				GPIO_ResetBits(GPIOE,GPIO_Pin_3);
				GPIO_SetBits(GPIOE,GPIO_Pin_4);
				break;
			default:break;
		}
		
}
/****************************************************
��������GetActiveState ���ص�ǰ����״̬ API���ⲿ����
�βΣ���
����ֵ����
�������ܣ�
****************************************************/
uint8_t GetActiveState(void)
{
		return activeState;
}

/**
* @brief  GetSuperState()���״̬����״̬ 1:�ػ� 2��׼����� 3�����ڳ�� 4:������
						
  * @param  
  * @param  
  * @retval None
  */
uint8_t GetSuperState(void)
{
		if(stateMachineSwitcher == 0)
			return 1;
		else
		{
			if(activeState == 7)
				return 4;
			else if(activeState == 6)
				return 3;
			else 
				return 2;
		}
}