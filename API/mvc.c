/**
  ******************************************************************************
  * @file    mvc.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "mvc.h"
#include "string.h"
#include "iDM100E.h"

#define STATE_SIZE            30		//�ܹ��ж��ٸ�state �����ж��ٸ�����
#define STATE_TRANSITION_SIZE 31  	//Ǩ����Ŀ�ĸ���
#define STATE_CONDITION_SIZE	7			//�������͵���Ŀ
#define STATE_CONDITION_CODE (STATE_CONDITION_SIZE+1)

conditionQueue mvcTriggerQueue;				/*�����������*/

struct message mvcTransitionMessage;	/*Ǩ���õ���Ϣʵ��*/

/****************************************************
��������transTable
����: ����model �����Ļ�����Ԫ
****************************************************/
TRANSACTION transactionTable;

/****************************************************
��������transactionTableUpdate��ʵʱ���У�
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У�transactionTable��Ϊ����������ݽṹ
transactionTableUpdate�͸�����������ʵʱ���µ�����У�ע��
�˺����в�����ȴ����������к�ʱ����������������뽫API׼��
�ã��Թ��˺������ã�
			�˺���ǣ��������֮���߼��ı�����߼��Ǻ�״̬���߼�
һ�£�״̬��ֻ�Ǳ��״̬֮����߼�����������֮����߼�Ҳ��Ҫ
���ģ����磬����ǲ����㹻
			�˴����ٲ���Эͬ�������д������Ϊִ��̫����
****************************************************/
void transactionTableUpdate()
{
		static uint8_t tmpTimeCounter;		/*��ʱֵ ����500ms�ļ���*/
		uint32_t accumulateElectricityOld;
		/*���ȸ���Ӳ������ֵ*/
	
		/*���µ�ѹ ���� ����*/
		transactionTable.voltage = GetVoltageValue();
		transactionTable.current = GetCurrentValue();
		transactionTable.w = GetWValue()*10;		/*0.1��*/
		
		transactionTable.rate = 1;
	
		transactionTable.cardStatus = GetCardStatus();
		transactionTable.lockStatus = GetLockStatus();
			
		transactionTable.keyValue =  GetKeyValue();
	
		transactionTable.chargeStatus =  GetSuperState();
	
		transactionTable.CCstate =	AdcValueToCCstate();
	
		transactionTable.rtcTime = GetTime();
	
		/*ȷ����ǰ���ģʽ*/
		if(MvcGetActiveState() == 2)
			transactionTable.chargeMode = 0;
		if(MvcGetActiveState() == 3)
			transactionTable.chargeMode = 2;
		if(MvcGetActiveState() == 4)
			transactionTable.chargeMode = 1;
		if(MvcGetActiveState() == 5)
			transactionTable.chargeMode = 3;
	
		/*�������ֵ ����������״̬���и��� ����˵ÿһ��״̬�µ�ˢ�����ݲ�һ��*/
		/*Ŀǰmvc״̬����״̬����ͨ��MvcGetActiveState()�õ�*/
		/*����״̬����״̬̫���ˣ�transactionStatusֻ��Ҫ����״̬�Ϳ�����*/
		/*ʵ����transactionStatus��super state����ʵ��4��״̬*/
		/*����----���----�ɷ�----����*/
	
		if(transactionTable.transactionStatus == 0)		/*��������ý׶�*/
		{
				if(transactionTable.cardStatus.status == 1)		/*������*/
						memcpy(&(transactionTable.transactionSponsor),\
									&(transactionTable.cardStatus.serialNum),\
									sizeof(transactionTable.cardStatus.serialNum));
				//else
				//		memset(&(transactionTable.transactionSponsor),0,sizeof(transactionTable.cardStatus.serialNum));
				
				/*������״̬�� ��׼����ֵ��ͣ���� ������״̬�²�����*/
				transactionTable.electricEnergy = transactionTable.w;
				
				switch(transactionTable.chargeMode)
				{
					case 0:/*����*/
						transactionTable.chargeMoney = transactionTable.chargeElectricity * 1;
						transactionTable.chargeTime =  transactionTable.chargeElectricity * 2;
						break;
					case 1:/*ʱ��*/
						transactionTable.chargeMoney = transactionTable.chargeTime / 2;
						transactionTable.chargeElectricity = transactionTable.chargeTime / 2;
						break;
					case 2:/*���*/
						transactionTable.chargeTime = transactionTable.chargeMoney / 3.0;
						transactionTable.chargeElectricity = transactionTable.chargeMoney * 1;
						break;
					default:break;
				}
		}
		if(transactionTable.transactionStatus == 1)		/*����ڳ��׶�*/
		{
				if(transactionTable.startTime.year == 0)
						transactionTable.startTime = GetTime();	/*��¼��ʼʱ��*/
				if(transactionTable.electricEnergy == 0)
						transactionTable.electricEnergy = GetWValue();	/*��¼��׼����ֵ*/
				
				/*�����ֵ*/
				accumulateElectricityOld = transactionTable.accumulateElectricity;
				transactionTable.accumulateElectricity = \
				transactionTable.w - transactionTable.electricEnergy;
				
				if((transactionTable.accumulateElectricity - accumulateElectricityOld) == 1)
				{
						transactionTable.accumulateMoney += transactionTable.rate;
				}
				
				tmpTimeCounter++;
				if(tmpTimeCounter >= 2)
				{
					transactionTable.accumulateTime++;	/*��2 X 500ms Ҳ����1sΪ��λ*/
					tmpTimeCounter = 0;
				}
				
				if(transactionTable.chargeMode == 0)		/*����ģʽ*/
				{
						if(transactionTable.accumulateElectricity >= \
							 transactionTable.chargeElectricity)
						{
								transactionTable.matchStatus = 1;			/*�ﵽ״̬��*/
						}
						else
						{
								transactionTable.matchStatus = 0;
						}
				}
				
				if(transactionTable.chargeMode == 1)		/*ʱ��*/
				{
						if(transactionTable.accumulateTime >= \
							 transactionTable.chargeTime * 60)
						{
								transactionTable.matchStatus = 1;			
						}
						else
						{
								transactionTable.matchStatus = 0;
						}
				}
				
				if(transactionTable.chargeMode == 2)		 /*���*/
				{
						if(transactionTable.accumulateMoney >= \
							 transactionTable.chargeMoney)
						{
								transactionTable.matchStatus = 1;			
						}
						else
						{
								transactionTable.matchStatus = 0;
						}
				}
		}
		if(transactionTable.transactionStatus == 2)		/*����ڽɷѽ׶�*/
		{
				
				if(transactionTable.endTime.year == 0)
						transactionTable.endTime = GetTime();		/*��¼����ʱ��*/
				if(transactionTable.cardStatus.status == 1)
				{
						if(transactionTable.cardStatus.rxWalletValue > transactionTable.accumulateMoney)
							transactionTable.moneyOK =1;

						memcpy(&(transactionTable.transactionPayer),\
									&(transactionTable.cardStatus.serialNum),\
									sizeof(transactionTable.cardStatus.serialNum));
				}
				else
					transactionTable.moneyOK = 0;
		}
		if(transactionTable.transactionStatus == 3)		/*����ڽ����׶�*/
		{
				/*���Ƚ�������Ϣ����*/
				/*����˴��������������*/
				memset(&transactionTable,0,sizeof(transactionTable));
		}
}

/****************************************************
��������mvcStateMachineSwitcher
������״̬���Ŀ��� 0���ر� 1������
****************************************************/
uint32_t mvcStateMachineSwitcher;

/****************************************************
��������mvcTimeBase 
��������1msΪ��λ��ʱ���� ����Լ��㵽49.71026��/���
������ÿһ��״̬��ʱ����ṩһ����׼����BMS��ʼʱ����
��BMS����ʱ��0��Ϊ״̬������ʱ������
****************************************************/
uint32_t mvcTimeBase;	

/****************************************************
��������mvcActiveState 
���������״̬
****************************************************/
uint8_t mvcActiveState;	

/****************************************************
��������mvcStateChanged 
������1��״̬�Ѿ��ı� 0��״̬û�иı�
��һ��������Ǩ�Ʊ��н���
���㶯������ʱ����н��У�һ���������㣩
****************************************************/
uint8_t mvcStateChanged;	

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
��������mvcStateTransitionTable  ״̬Ǩ�Ʊ�
��������n��stateTransitionItem��ɣ����������е�״̬
Ǩ�ƣ������߼��ϵĺ�ʱ���ϵ� 
----<ԭ״̬���������룬Ŀ��״̬������>-----
****************************************************/
stateTransitionItem mvcStateTransitionTable[STATE_TRANSITION_SIZE] = 
{
		{1, 	 0xe1,  2,   0},			/*ȷ������ģʽ*/
		{2, 	 0xe2,  1,   0},			/*����*/
		{2, 	 0xe3,  6,   0},			/*ȷ��*/
		{6, 	 0xe2,  2,   0},			/*����*/
		{6, 	 0xe8,  2,   0},			/*����*/
		
		{6, 	 0xe7, 10, 	 0xc1},		/*���� ��״̬��*/
		{10, 	 0xe4, 14, 	 0xc2},		/*�ر�״̬��*/
		{10, 	 0xe5, 14, 	 0xc2},		/*�ر�״̬��*/
		{10, 	 0xe9, 14, 	 0xc2},	
		{10, 	 0xD2, 14, 	 0xc2},	
		{10, 	 0xD3, 14, 	 0xc2},	
		{10, 	 0xD4, 14, 	 0xc2},	
		{10, 	 0xD5, 14, 	 0xc2},
		{10, 	 0xD6, 14, 	 0xc2},		/*������ֹ*/
		{14, 	 0xe6, 22, 	 0xc3},		/*�۷� ����*/
		{22, 	 0xe8,  1,   0xc4},		/*��¼�˴γ�����*/
		
		{1, 	 0xea,  3,   0},			/*ȷ�����ģʽ*/
		{3, 	 0xe2,  1,   0},			/*����*/
		{3, 	 0xe3,  6,   0},			/*ȷ��*/
		{6, 	 0xe2,  3,   0},			/*����*/
		{6, 	 0xe8,  3,   0},			/*����*/
		
		{1, 	 0xeb,  4,   0},			/*ȷ��ʱ��ģʽ*/
		{4, 	 0xe2,  1,   0},			/*����*/
		{4, 	 0xe3,  6,   0},			/*ȷ��*/
		{6, 	 0xe2,  4,   0},			/*����*/
		{6, 	 0xe8,  4,   0},			/*����*/
		
		{1, 	 0xec,  5,   0},			/*ȷ���Զ�ģʽ*/
		{5, 	 0xe2,  1,   0},			/*����*/
		{5, 	 0xd0,  1,   0},			/*����*/
		{5, 	 0xe8,  1,   0},			/*����*/
		{5, 	 0xe7,  10,  0xc1}		/*ȷ��*/
};

/****************************************************
��������stateTimeItem  ״̬ʱ���
��������¼��״̬���п��ܵ�ʱ��� ͨ�����������ж�
״̬�ǲ��ǳ�ʱ�������Ĵ����ǲ���̫�࣬�涨ʱ������û��
���ĳһ�����ȵȣ���Ǩ�Ʊ��ʱ�����ԡ�
****************************************************/
typedef struct 
{
	uint32_t firstTime;		//״̬�ʼʱ��
	uint32_t latestTime;	//״̬���һ��ʱ��
	uint32_t duration;		//״̬���һ�γ���ʱ��
	uint32_t times;				//״̬�ܹ�����
}stateTimeItem;

/****************************************************
��������mvcStateTimeTable  ״̬ʱ���
��������n��stateTimeItem��ɣ����������е�״̬
��ʱ��
****************************************************/
stateTimeItem mvcStateTimeTable[STATE_SIZE];

/****************************************************
��������conditionsQueue  ��������
��������ĳ��state��������£����в�����������֮����
��һ�����У�����Ϊ�����Ĳ���Ƶ���п��ܴ���Ǩ�Ƹ��µ�Ƶ��
Ϊ�˷�ֹ���������������Բ����˶���
�ڷ����˳ɹ���Ǩ��֮��Ҫ�Ѷ�����ʣ����������㣬��ֹ
Ӱ����һ��״̬�������߼�����
****************************************************/
conditionQueue mvcTriggerQueue;

/****************************************************
��������tableInit
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ʼ��״̬���ĸ���������ص�
״̬���ʼ��״̬ 
****************************************************/
void MvcTableInit()
{
	uint8_t counter;
	mvcStateMachineSwitcher = 0; //�ر�״̬��
	mvcTimeBase = 0;			  //ʱ��������
	mvcActiveState = 1;		  //��ʼ״̬
	for(counter = 0; counter < STATE_SIZE; counter++)
	{
		mvcStateTimeTable[counter].firstTime = 0;
		mvcStateTimeTable[counter].latestTime = 0;
		mvcStateTimeTable[counter].duration = 0;
		mvcStateTimeTable[counter].times = 0;
	}
	mvcTriggerQueue.size = 0;	  //�����������
}

/****************************************************
��������MvcStateMachineOpen
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ʼ��״̬���ĸ���������ص�
״̬���ʼ��״̬ 
****************************************************/
void MvcStateMachineOpen()
{
	//MvcTableInit();
	mvcStateMachineSwitcher = 1; //��״̬��
}

/****************************************************
��������MvcStateMachineClose
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У���ʼ��״̬���ĸ���������ص�
״̬���ʼ��״̬ 
****************************************************/
void MvcStateMachineClose()
{
	mvcStateMachineSwitcher = 0; //�ر�״̬��
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
void MvcStateTransitionUpdate()
{
	uint8_t tCounter = 0;
	uint8_t cCounter = 0;
	
	if(mvcStateMachineSwitcher == 0)	//˵��״̬�����ڹر�״̬
		return;
		
	//���Ȳ����������� ����� ���� ���û�� return
	if(mvcTriggerQueue.size == 0)
		return; 
		
	//�Ա����б��� Ѱ����mvcActiveState��ͬ����Ŀ
	//Ҳ����˵ ����mvcActiveState�Ĵ�����������ʲô
	for(tCounter = 0; tCounter < STATE_TRANSITION_SIZE; tCounter++)
	{
		if(mvcStateTransitionTable[tCounter].sourceState == mvcActiveState)
		{	//�ҵ�Դ״̬==mvcActiveState��Ǩ����
			for(cCounter = 0 ; cCounter < mvcTriggerQueue.size; cCounter++)
			{	
				//�����������������һһ�Ա�
				if(mvcStateTransitionTable[tCounter].triggerCondition == \
					mvcTriggerQueue.queue[cCounter])
				{
					//ִ�д�����Ķ���....
					//���ȸ��Ļ״̬
					mvcActiveState = mvcStateTransitionTable[tCounter].targetState;
					//��������������
					mvcTriggerQueue.size = 0;
					//״̬�ı��־λ
					mvcStateChanged = 1;
					//ִ��ĳЩ����....
					if(mvcStateTransitionTable[tCounter].action != 0) /*ȷʵ����Ч�Ķ�������*/
					{
							mvcTransitionMessage.destinationAddress = 5;
							mvcTransitionMessage.cmd = 2;
							mvcTransitionMessage.parameter_1 = mvcStateTransitionTable[tCounter].action;
							messagePost(mvcTransitionMessage);
					}
				}
				else	/*Ǩ������û�б�����*/
				{
					//֪ͨϵͳ���õ��������������ڵ���
				}
				if(mvcStateChanged == 1)
					break;
			}
		}
		if(mvcStateChanged == 1)
				break;
	}
	
	/*��������֮��û�����mvcActiveState�����������㣬Ҳ����˵
	������Щ�ڶ�������������������mvcActiveState�ģ���ô��Щ
	��������״̬����˵�����壬Ӧ�ñ�����*/
	/*��������Ǩ�Ʒ��� ҲӦ�ñ�����*/
	
	mvcTriggerQueue.size = 0;
	
	/*��Ǩ��ִ��֮��״̬�п��ܱ��ı��ˣ�����Ҫ�����Եĸ��º��״̬*/
	/*�����Ǵ������� ��� �ɷ� ���ǽ��� ״̬�е�һ�� �Ա���Ϊֵ�ĸ���
	�ṩ����*/
	MvcGetSuperState();
}

/****************************************************
��������stateTimeUpdate��1ms��Ϣ������
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У�ÿ1ms����һ��mvcTimeBase
�жϻ��״̬�Ƿ�ı���mvcStateChanged����
���������µ�״̬����ô������״̬��duration
���������µ�״̬����ô����times++
ֻҪfirstTimeΪ0����ô�ͼ�¼ΪmvcTimeBase
���ڻ��״̬��duration++����¼��
���ڻ��״̬latestTime��¼ΪmvcTimeBase
ʱ�����Ǩ�Ʊ��ʱ�����ԣ�
1.������ÿһ��״̬����ϸʱ������
****************************************************/
void MvcStateTimeUpdate()
{
	uint8_t counter;			/*��ͨ����*/
	
	if(mvcStateMachineSwitcher == 0)	//˵��״̬�����ڹر�״̬
		return;

	mvcTimeBase++;					//ÿ1ms��һ
	if(mvcStateChanged)    //״̬������Ǩ��,״̬�ı���
	{
		mvcStateTimeTable[mvcActiveState].duration = 0;
		mvcStateTimeTable[mvcActiveState].times++;

		//�����״̬��firstTimeΪ0 
		if(mvcStateTimeTable[mvcActiveState].firstTime == 0)
		{
			//����ʱ���
			mvcStateTimeTable[mvcActiveState].firstTime = mvcTimeBase;
		}
		mvcStateChanged = 0;	//����־λ���� ����ʱ�������˵�Ѿ�����
	}
	
	mvcStateTimeTable[mvcActiveState].duration++; //״̬��������
	mvcStateTimeTable[mvcActiveState].latestTime = mvcTimeBase; //��������ʱ��
	
}
/****************************************************
��������mvcTimeOutTable 
������ͣ����ÿһ��״̬��timeOutʱ�� �˴�����STATE_SIZE��ʼ��ֵ
			��������ĳ���
****************************************************/
const uint32_t mvcTimeOutTable[STATE_SIZE] = 
{
 1000,1000,1000,1000,1000,1000,1000,10,10,10,
 1000,10,250,10,10,250,10,10,10,10,
 10,10,10000,10,10,250,10,10,10,10
};
/****************************************************
��������mvcConditionTable 
���������ݿ�״̬ ������ֵ ���״̬��״̬ ��ֵ ӳ�䵽 ��������
****************************************************/
const uint8_t mvcConditionTable[24][STATE_CONDITION_CODE] = 
{//card  money   status  stateM  key_value   time   gun   code
	{0,      0,      0,       0 ,   0x03,       0,     0,   0xE1 },/*������*/
	{0,      0,      0,       0 ,   0x04,       0,     0,   0xEA },/*����*/
	{0,      0,      0,       0 ,   0x05,       0,     0,   0xEB },/*ʱ���*/
	{0,      0,      0,       0 ,   0x06,       0,     0,   0xEC },/*�Զ���*/
	{0,      0,      0,       0 ,   0x07,       0,     0,   0xED },/*user��*/
	{0,      0,      0,       0 ,   0x08,       0,     0,   0xEE },/*root��*/
	{0,      0,      0,       0 ,   0x09,       0,     0,   0xEF },/*deve��*/
	{0,      0,      0,       0 ,   0x02,       0,     0,   0xE2 },/*���ؼ�*/
	{0,      0,      0,       0 ,   0x01,       0,     0,   0xE3 },/*ȷ����*/
	{0,      0,      1,       3 ,      0,       0,   	 0,   0xE4 },/*���ڳ�� match��*/
	{0,      0,      2,       0 ,      0,       0,     0,   0xE5 },/**/
	{1,      1,      0,       0 ,      0,       0,     0,   0xE6 },/*������ ǮҲ��*/
	{0,      0,      0,       0 ,   0x01,       0,  0x01,   0xE7 },/*������ ȷ����*/
	{0,      0,      0,       0 ,   0x02,       0,  0x01,   0xE8 },/*������ ���ؼ�*/
	{1,      0,      0,       0 ,   0x01,       0,     0,   0xD1 },/*������ ȷ��*/
	{1,      0,      0,       0 ,   0x02,       0,     0,   0xD0 },/*������ ����*/
	{0,      0,      0,     0x03,   0x02,       0,     0,   0xD6 },/*����� ����*/
	{1,      0,      0,       0 ,      0,       0,     0,   0xE7 },/*������*/
	{0,      0,      0,       0 ,      0,      22,     0,   0xE8 },/*22��ʱ*/
	{0,      0,      0,       4 ,      0,       0,     0,   0xE9 },/**/
	{0,      0,      1,       1 ,      0,       0,     0,   0xD2 },/**/
	{0,      0,      1,       2 ,      0,       0,     0,   0xD3 },/**/
	{0,      0,      1,       3 ,      0,       0,     0,   0xD4 },/**/
	{0,      0,      1,       4 ,      0,       0,     0,   0xD5 },/**/
};
/****************************************************
��������conditionMask
���������ݿ�״̬ ������ֵ ���״̬��״̬ ��ֵ ӳ�䵽 ��������
****************************************************/
const uint8_t conditionMask[23][STATE_CONDITION_SIZE] = 
{//card  money   status  stateM  key_value   time      gun
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S0*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S1*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S2*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S3*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S4*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0xff},/*S5*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0xff},/*S6*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/*S9���Խ���*/
	{0,      0,   0xff,     0xff,   0xff,       0  ,      0},/*S10*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0xff, 0xff,      0,       0 ,     0,        0,        0},/*S14*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0xff,0xff,      0,       0 ,      0xff,    0  ,      0},/*S18*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0xff,     0}/*S22*/
};
/****************************************************
��������conditionUpdate �����ռ���10ms��Ϣ������
�βΣ���
����ֵ����
�������ܣ��ڴ˺����У�ÿ1ms����һ������ ���Ƕ���
ʱ��Ҳ�ǿ��Բ���Ǩ�������ģ��ڴ˺�����ҲҪ��ʱ�俼��
��˿��ܲ����������� ���߸���
�ܽ����2�� :�߼�Ǩ�ƺ�ʱ��Ǩ�ƣ�
****************************************************/
void MvcConditionUpdate()
{
	uint8_t cCounter;		//�����ñ���
	uint8_t tCounter;		//ʱ���ñ���
	uint8_t startFlag;	//��Ч��־
	uint8_t resultFlag;
	uint8_t timeOutFlag;
	uint8_t conditionArray[7];		/*�����б�*/
	
	if(mvcStateMachineSwitcher == 0)	//˵��״̬�����ڹر�״̬
		return;
	
	/*��ȡʵʱ����*/
	conditionArray[0] = transactionTable.cardStatus.status;		/*ȷ��״̬*/
	conditionArray[1] = transactionTable.moneyOK;							/*��һ������Ч״̬*/
	conditionArray[2] = transactionTable.matchStatus;					/*0δ�ﵽ 1�ﵽ*/
	conditionArray[3] = transactionTable.chargeStatus;				/*���׮״̬������״̬ ������*/
	conditionArray[4] = transactionTable.keyValue;
	/*conditionArray[5] ������*/
	conditionArray[6] = transactionTable.CCstate;							/*0δ���� 1����*/
	
	/*ɨ��ʱ�����������mvcActiveState�ģ�*/
	/*ʱ����������ʹduration����ʱ��̫���ˣ�Ҳ�����ǲ�����״̬֮��
	��ʱ��̫���ˣ���������firsttime��ĳ��һ״̬������ʱ�䣬ʹ����
	���Ƚ������˳���ʱ��duration������������������ݽṹ��ʾ��
	������Ƚϸ��ӣ���������Գ���Ĳ��죬��������ʵ�֡�duration��
	���ñ���ʾ��*/
	
	/*���ȼ��durition�����*/
	if(mvcStateTimeTable[mvcActiveState].duration >= mvcTimeOutTable[mvcActiveState])
	{
			//������ʱ����������...
			conditionArray[5] = mvcActiveState;
	}
	
	/*�õ�����ʵʱ����֮�� ����˵ Ӧ�ú���������յõ���������*/
	/*�� ����-----> �������� ӳ��Ĺ���*/
	/*������ÿһ��״̬������ע�������ǲ�ͬ�� �ص��ǲ�ͬ�ģ���������Ի״̬
		�����������봦�� ��ֹ���ų���*/
	/*��Ի״̬���������˵õ�����*/
	for(cCounter = 0; cCounter < STATE_CONDITION_SIZE; cCounter++)
	{
			conditionArray[cCounter] = conditionArray[cCounter] & \
																 conditionMask[mvcActiveState][cCounter];
	}
	/*��������--->��������ӳ��� �õ�����*/
	for(cCounter = 0; cCounter < 24; cCounter++)
	{
		if(conditionArray[0] == mvcConditionTable[cCounter][0] && \
		   conditionArray[1] == mvcConditionTable[cCounter][1] && \
		   conditionArray[2] == mvcConditionTable[cCounter][2] && \
			 conditionArray[3] == mvcConditionTable[cCounter][3] && \
			 conditionArray[4] == mvcConditionTable[cCounter][4] && \
			 conditionArray[5] == mvcConditionTable[cCounter][5] && \
		   conditionArray[6] == mvcConditionTable[cCounter][6] )
		{
			//����������д��
			mvcTriggerQueue.queue[mvcTriggerQueue.size] = mvcConditionTable[cCounter][STATE_CONDITION_SIZE];
			mvcTriggerQueue.size++;
			break; //�ҵ�һ���������˳� ��Ϊ�������������
		}
	}
	

	//�ټ��������ֿ�״̬��ʱ���������... ��ʱ����Ҫ
#if 0
	if(mvcActiveState == 2)
	{
			if((mvcStateTimeTable[2].latestTime-mvcStateTimeTable[1].firstTime)> 5000)
			{
					ConditionInsert(&mvcTriggerQueue, 0xE1); /*���������0xe1��ʼ*/
			}
	}
	//..

	
	/*�����ռ�can���ձ��ĵ����*/
	while(canmvcTriggerQueue.size--)
	{
			ConditionInsert(&mvcTriggerQueue, canmvcTriggerQueue.queue[canmvcTriggerQueue.size-1]);
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
									ConditionInsert(&mvcTriggerQueue, 0xEB);
								
									/*�رմ˴μ�ʱ*/
									memset(messageTimeTable,0,sizeof(messageTimeTable));
							}
					}
			}
			else		/*�ɹ�������*/
			{
					/*֪ͨ״̬�� ���ͳɹ������ź�*/
					ConditionInsert(&mvcTriggerQueue, 0xEC);
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
void MvcConditionInsert(conditionQueue * pC ,uint8_t conditionCode)
{
		pC->queue[pC->size] = conditionCode; 
		pC->size++;
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
void MvcActionExecutor(struct message actionMessage)		/*��������ǧ����� cmd���Ǳ���*/
{
		switch(actionMessage.parameter_1)
		{
			case 0xc1:
				LockSet(LOCK);
				stateMachineOpen();
				break;
			case 0xc2:
				stateMachineClose();
				break;
			case 0xc3:
				WalletWrite(transactionTable.cardStatus.rxWalletValue - transactionTable.accumulateMoney);
				LockSet(UNLOCK);
				usart1SendByte(0xba);
				break;
			case 0xc4:
				/*��¼�˴γ�����*/
				break;
			default:break;
		}
		
}
/****************************************************
��������GetmvcActiveState ���ص�ǰ����״̬ API���ⲿ����
�βΣ���
����ֵ����
�������ܣ�
****************************************************/
uint8_t MvcGetActiveState(void)
{
		return mvcActiveState;
}

/****************************************************
��������MvcGetSuperState ���ص�ǰ�����ĺ��״̬ 
				�˺������ݵ��Ǵ��������ó����״̬
				����������Ǻ��״̬ �ұ���΢��״̬ ���������ϵ
�βΣ���
����ֵ����
�������ܣ�
****************************************************/
uint8_t subordinateTable[10][2] = 
{
		{0, 1},
		{0, 2},
		{0, 6},
		{1, 10},
		{2, 14},
		{2, 18},
		{3, 22},
};
uint8_t MvcGetSuperState(void)
{
		uint8_t counter;
		for(counter = 0; counter < 7; counter++)
		{
					if(subordinateTable[counter][1] == mvcActiveState)
					{
							  transactionTable.transactionStatus = subordinateTable[counter][0];
								return subordinateTable[counter][0];
					}
		}
}

/****************************************************
��������MvcGetTransation ���ص�ǰ����ṹ�� ���ⲿ����API
�βΣ���
����ֵ����
�������ܣ�
****************************************************/
TRANSACTION MvcGetTransation()
{
		return transactionTable;
}
/****************************************************
��������MvcSetChargeValue ���ó��ֵ ���ⲿ����API
�βΣ���
����ֵ����
�������ܣ�0���� 1ʱ�� 2��� 3�Զ�
****************************************************/
void MvcSetChargeValue(uint32_t value)
{
		switch(transactionTable.chargeMode)
		{
			case 0:	/*���� 0.1��*/
				transactionTable.chargeElectricity = value;	
				break;
			case 1:	/*ʱ�� ��*/
				transactionTable.chargeTime = value;
				break;
			case 2:	/*��� ��*/
				transactionTable.chargeMoney = value;
				break;
			default:break;
		}
}
/****************************************************
��������MvcSetChargeMode ���ó��ֵ ���ⲿ����API
�βΣ���
����ֵ����
�������ܣ�0���� 1ʱ�� 2��� 3�Զ�
****************************************************/
void MvcSetChargeMode(uint8_t mode)
{
	transactionTable.chargeMode = mode;
}