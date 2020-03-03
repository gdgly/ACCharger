/* Includes ------------------------------------------------------------------*/
#include "pgn.h"

/* Exported types ------------------------------------------------------------*/
/*��������bmsͨ�ŵĻ���������Ҫ��ܶ����������ϵ*/
extern pgnGroup pgnValue;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define NUMBER_OF_MESSAGE 22
/* Exported functions ------------------------------------------------------- */

/*��������messageTable
  �������˱��¼�����б��ĵ�ȫ����Ϣ������������ѯ*/
typedef struct
{
	uint8_t identifier;	//����
	uint16_t pgn;		//������
	uint8_t priority;	//���ȼ�
	uint8_t length;		//����
	uint16_t cycle;		//����
}messageItem;

/*������յ���can����Ϣ��ͨ������PGN����ӳ�䵽������
֪���˴��žͿ��Ը���conditionsUpdate���յ���ʲô֡��
conditionUpdateͨ������ӳ�䵽�����б��Խ���״̬Ǩ��
���� ÿһ��CAN�������ݲ�ͬ�Ͳ����ܽ�����CHM��BHM����
�򵥵�������������ݣ����Է�Ϊ�������쳣����˻�Ҫ��
ϸ��Ϊ����������쳣������ֻ������*/
/*�е�can��Ϣ����������Ӱ��Ǩ�Ƶģ��е���Ӱ�죬����
Ӧ�ð�CAN��Ϣ����+CAN��Ϣ���� һ������Ϣ�з��͸�����
����*/

messageItem messageTable[NUMBER_OF_MESSAGE] = {
{CHM, 0x2600, 6, 3, 250},
{BHM, 0x2700, 6, 2, 250},

{CRM, 0x0100, 6, 8, 250},
{BRM, 0x0200, 7, 11, 250},

{BCP, 0x0600, 7, 13, 500},
{CTS, 0x0700, 6, 7, 500},
{CML, 0x0800, 6, 8, 250},
{BRO, 0x0900, 4, 1, 250},
{CRO, 0x0a00, 4, 1, 250},

{BCL, 0x1000, 6, 5, 50},
{BCS, 0x1100, 7, 9, 250},
{CCS, 0x1200, 6, 8, 50},
{BSM, 0x1300, 6, 7, 250},
{BMV, 0x1500, 7, 0, 10000},
{BMT, 0x1600, 7, 0, 10000},
{BSP, 0x1700, 7, 0, 10000},
{BST, 0x1900, 4, 4, 10},
{CST, 0x1A00, 4, 4, 10},

{BSD, 0x1C00, 6, 7, 250},
{CSD, 0x1D00, 6, 8, 250},

{BEM, 0x1E00, 2, 4, 250},
{CEM, 0x1F00, 2, 4, 250}
};
/**
  * @brief  PgnGroupInit��ʼ�����в�����Ĭ��״̬
  * @param  
  * @retval 
  */
void PgnGroupInit()
{
		memset(&pgnValue,0,sizeof(pgnValue));
}
/**
  * @brief  FormulaOnPgnGroup����pgnGroup�ϵļ��� �����Ǳ�����֮��Ĺ�ϵ
		ͨ��ʵʱ����ķ�ʽ�������ڼ��̵�ʱ���ڸ��±��״̬���Ӷ�ʹ������Ч ͬʱ
		Ҳ������ݴ���
  * @param  
  * @retval 
  */
void FormulaOnPgnGroup()
{
		if(pgnValue.SPN2565 != 0)				/*����汾�Ų�ΪV0.0 ����V1.0 V1.1*/
		{
				pgnValue.SPN2560 = 0xAA;		/*�������ʶ��*/
		}
		if(1/*����ȫ�����ڿ�״̬*/)
		{
				pgnValue.SPN2830 = 0xaa;		/*������״̬׼������*/
		}
}
/**
  * @brief  PgnToCode pgn������
  * @param  None
  * @retval None
  */
uint8_t PgnToCode(uint8_t pgn)
{
		switch(pgn)
		{
			case BHM:return 0xB1;
			case BRM:return 0xB2;
			case BCP:return 0xB3;
			case BRO:return 0xB4;
			case BCS:return 0xB5;
			case BCL:return 0xB6;
			case BSM:return 0xB7;
			case BST:return 0xB8;
			case BSD:return 0xB9;
			default:break;
		}
}

/*���ļ�¼���� ���н��յ��ı��Ķ����������¼ ͬʱת���ɱ���*/
conditionQueue canTriggerQueue;

/**
  * @brief  NormalMessageParse �����յ���֡������������ �˺���ֻ�ǽ����յ���can֡�е�
	�������뵽�������У�û���κμ�����ݴ�ֻ��������Ĳ������ܡ�
  * @param  CanRxMsg
  * @retval None
  */
void NormalMessageParse(CanRxMsg rxMessage)
{
	uint32_t extId;
	uint32_t pgn;
	if (rxMessage.IDE == CAN_ID_STD)	/*�õ���֡�Ǳ�׼֡ ���󷵻�*/
  {
			return;
  }
	
	extId = rxMessage.ExtId;			/*�õ�ʵ��IDֵ*/
	pgn = (extId >> 16) & 0x000000ff; /*��ID����ȡpgn*/

	/*ת��Ϊ�������� ѹ����� �Թ�conditionUpdate����*/
	canTriggerQueue.queue[canTriggerQueue.size] = PgnToCode(pgn);
	canTriggerQueue.size++;
	
	switch(pgn)		/*����PGN��������Ӧ�Ĳ�����*/
	{
		case BHM:	pgnValue.SPN2601 = ((rxMessage.Data[1])<<8)|\
																	(rxMessage.Data[0]);		/*����������ܵ�ѹ*/
							/*���ȸ�������*/
							
							/*Ȼ����¶�ʱ�� ��¼����ʵʱ��*/
							//...
							break;
		case BRM:	pgnValue.SPN2565 = ((rxMessage.Data[2])<<16)|\
																 ((rxMessage.Data[1])<<8)|\
																  (rxMessage.Data[0]);
							pgnValue.SPN2566 =  (rxMessage.Data[3]);
							pgnValue.SPN2567 = ((rxMessage.Data[5])<<8)|\
																 ((rxMessage.Data[4]));
							pgnValue.SPN2568 = ((rxMessage.Data[7])<<8)|\
																 ((rxMessage.Data[6]));
							break;
		case BRO:	pgnValue.SPN2829 = ((rxMessage.Data[0]));break;
		case BCL:	pgnValue.SPN3072 = ((rxMessage.Data[1])<<8)|\
																 ((rxMessage.Data[0]));
							pgnValue.SPN3073 = ((rxMessage.Data[3])<<8)|\
																 ((rxMessage.Data[2]));
							pgnValue.SPN3074 = ((rxMessage.Data[4]));
							break;
		case BSM:	pgnValue.SPN3085 = ((rxMessage.Data[0]));
							pgnValue.SPN3086 = ((rxMessage.Data[1]));
							pgnValue.SPN3087 = ((rxMessage.Data[2]));
							pgnValue.SPN3088 = ((rxMessage.Data[3]));
							pgnValue.SPN3089 = ((rxMessage.Data[4]));
							pgnValue.SPN3090 = ((rxMessage.Data[5])&0x03);
							pgnValue.SPN3091 = ((rxMessage.Data[5]>>2)&0x03);
							pgnValue.SPN3092 = ((rxMessage.Data[5]>>4)&0x03);
							pgnValue.SPN3093 = ((rxMessage.Data[5]>>6)&0x03);
							pgnValue.SPN3094 = ((rxMessage.Data[6])&0x03);
							pgnValue.SPN3095 = ((rxMessage.Data[6]>>2)&0x03);
							pgnValue.SPN3096 = ((rxMessage.Data[6]>>4)&0x03);
							break;
		case BST:	pgnValue.SPN3511 = ((rxMessage.Data[0]));
							pgnValue.SPN3512 = ((rxMessage.Data[2])<<8)|\
																 ((rxMessage.Data[1]));
							pgnValue.SPN3513 = ((rxMessage.Data[3]));
							break;
		case BSD:	pgnValue.SPN3601 = ((rxMessage.Data[0]));
							pgnValue.SPN3602 = ((rxMessage.Data[2])<<8)|\
																 ((rxMessage.Data[1]));
							pgnValue.SPN3603 = ((rxMessage.Data[4])<<8)|\
																 ((rxMessage.Data[3]));
							pgnValue.SPN3604 = ((rxMessage.Data[5]));
							pgnValue.SPN3605 = ((rxMessage.Data[6]));
							break;
		case BEM:	pgnValue.SPN3901 = ((rxMessage.Data[0])&0x03);
							pgnValue.SPN3902 = ((rxMessage.Data[0]>>2)&0x03);
							pgnValue.SPN3903 = ((rxMessage.Data[1])&0x03);
							pgnValue.SPN3904 = ((rxMessage.Data[1]>>2)&0x03);
							pgnValue.SPN3905 = ((rxMessage.Data[2])&0x03);
							pgnValue.SPN3906 = ((rxMessage.Data[2]>>2)&0x03);
							pgnValue.SPN3907 = ((rxMessage.Data[3])&0x03);
							break;
		
	}
}
/**
  * @brief  LongMessageParse �����յ���֡������������ �˺���ֻ�ǽ����յ���can֡�е�
	�������뵽�������У�û���κμ�����ݴ�ֻ��������Ĳ������ܡ�
  * @param  CanRxMsg
  * @retval None
  */
void LongMessageParse(struct linkStatus * pD)
{
	uint8_t temp[40],i,j;
	uint32_t pgn;
	
	pgn = pD->PGNCode;
	
	for(i = 0, j = 0; i < pD->numOfPackage; i++, j = j + 7)
	{
			memcpy(temp+j,((pD->buffer[i]).data + 1),7);
	}
	
	/*ת��Ϊ�������� ѹ����� �Թ�conditionUpdate����*/
	canTriggerQueue.queue[canTriggerQueue.size] = PgnToCode(pgn);
	canTriggerQueue.size++;
	
	switch(pgn)		/*����PGN��������Ӧ�Ĳ�����*/
	{
		case BCP:	pgnValue.SPN2816 = ((temp[1])<<8)|\
																 ((temp[0]));
							pgnValue.SPN2817 = ((temp[3])<<8)|\
																 ((temp[2]));
							pgnValue.SPN2818 = ((temp[5])<<8)|\
																 ((temp[4]));
							pgnValue.SPN2819 = ((temp[7])<<8)|\
																 ((temp[6]));
							pgnValue.SPN2820 = ((temp[8]));
							pgnValue.SPN2821 = ((temp[10])<<8)|\
																 ((temp[9]));
							pgnValue.SPN2822 = ((temp[12])<<8)|\
																 ((temp[11]));
							break;
		case BCS:	pgnValue.SPN3075 = (temp[1] << 8)| temp[0];
							pgnValue.SPN3076 = (temp[3] << 8)| temp[2];
							pgnValue.SPN3077 = (temp[5] << 8)| temp[4];
							pgnValue.SPN3078 =  temp[6];
							pgnValue.SPN3079 = (temp[8] << 8)| temp[7];
		case BMV:	
							break;
		case BMT:	
							break;
		case BSP:	
							break;
	}
}

/**
  * @brief  ActionHandle (�ڶ���)���𽫲������е����ݷ��ͳ�ȥ �˺���
		ֻ�ǵ����ؽ�can�������е���������can֡�У�Ȼ���ͣ�û���κμ����
		�ݴ���ƣ�����������Ĳ������ܡ�
  * @param  
  * @retval None
  */
void ActionHandle(struct message txMessage)
{
		uint8_t pgn;
		CanTxMsg canTxMessage;
	
		pgn = txMessage.parameter_1;
	
		switch(pgn)
		{
			case 0xc1:	//canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CHM<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 3;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN2600;				//д������
								canTxMessage.Data[1] = pgnValue.SPN2600 >> 8;		//д������
								canTxMessage.Data[2] = pgnValue.SPN2600 >> 16;  //д������
			
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			
			case 0xc2: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CRM<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN2560;				//д������
								canTxMessage.Data[1] = pgnValue.SPN2561;		//д������
								canTxMessage.Data[2] = pgnValue.SPN2561 >> 8;  //д������
								canTxMessage.Data[3] = pgnValue.SPN2561 >> 16;
								canTxMessage.Data[4] = pgnValue.SPN2561 >> 24;
								canTxMessage.Data[5] = pgnValue.SPN2562; 
								canTxMessage.Data[6] = pgnValue.SPN2562 >> 8;
								canTxMessage.Data[7] = pgnValue.SPN2562 >> 16;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			case 0xc3: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CTS<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN2823[0];			/*BCDʱ��*/
								canTxMessage.Data[1] = pgnValue.SPN2823[1];	
								canTxMessage.Data[2] = pgnValue.SPN2823[2]; 
								canTxMessage.Data[3] = pgnValue.SPN2823[3];
								canTxMessage.Data[4] = pgnValue.SPN2823[4];
								canTxMessage.Data[5] = pgnValue.SPN2823[5]; 
								canTxMessage.Data[6] = pgnValue.SPN2823[6];
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			case 0xc4: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CML<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN2824;	
								canTxMessage.Data[1] = pgnValue.SPN2824 >> 8;		
								canTxMessage.Data[2] = pgnValue.SPN2825; 
								canTxMessage.Data[3] = pgnValue.SPN2825 >> 8;
								canTxMessage.Data[4] = pgnValue.SPN2826;
								canTxMessage.Data[5] = pgnValue.SPN2826 >> 8; 
								canTxMessage.Data[6] = pgnValue.SPN2827;
								canTxMessage.Data[7] = pgnValue.SPN2827 >> 8;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			case 0xc5: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CRO<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN2830;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								
								/*CROһ������ �Ϳ�ʼ�ȴ�BCL BCS*/
								messageTimeTableSet(0);
								messageTimeTableSet(1);
			
								break;
			case 0xc6: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CCS<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN3081;
								canTxMessage.Data[1] = pgnValue.SPN3081 >> 8;
								canTxMessage.Data[2] = pgnValue.SPN3082;
								canTxMessage.Data[3] = pgnValue.SPN3082 >> 8;
								canTxMessage.Data[4] = pgnValue.SPN3083;
								canTxMessage.Data[5] = pgnValue.SPN3083 >> 8; 
								canTxMessage.Data[6] = pgnValue.SPN3929 & 0x03;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
			
								/*CROһ������ �Ϳ�ʼ�ȴ�BCL BCS BSM ���ƻ��˳��������� ����*/
								messageTimeTableSet(0);
								messageTimeTableSet(1);
								messageTimeTableSet(2);
								
								break;
			case 0xc7: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CST<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN3521;
								canTxMessage.Data[1] = pgnValue.SPN3522;
								canTxMessage.Data[2] = pgnValue.SPN3522 >> 8;
								canTxMessage.Data[3] = pgnValue.SPN3523;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			case 0xc8: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CSD<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN3611;
								canTxMessage.Data[1] = pgnValue.SPN3611 >> 8;
								canTxMessage.Data[2] = pgnValue.SPN3612;
								canTxMessage.Data[3] = pgnValue.SPN3612 >> 8;
								canTxMessage.Data[4] = pgnValue.SPN3613;
								canTxMessage.Data[5] = pgnValue.SPN3613 >> 8; 
								canTxMessage.Data[6] = pgnValue.SPN3613 >> 16;
								canTxMessage.Data[7] = pgnValue.SPN3613 >> 24;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			case 0xc9: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CEM<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//����֡
								canTxMessage.IDE = CAN_ID_EXT;			//��չ֡
								canTxMessage.DLC = 8;							//���ݳ���
								canTxMessage.Data[0] = pgnValue.SPN3921;
								canTxMessage.Data[1] = pgnValue.SPN3922 & 0x03;
								canTxMessage.Data[1] |= (pgnValue.SPN3923 << 2) & 0x0c;
								canTxMessage.Data[2] = pgnValue.SPN3924 & 0x03;
								canTxMessage.Data[2] |= (pgnValue.SPN3925 << 2) & 0x0c;
								canTxMessage.Data[2] |= (pgnValue.SPN3926 << 4) & 0x30;
								canTxMessage.Data[3] = pgnValue.SPN3927 >> 8;
								
								CanQueueTxEnqueue(canTxMessage);	/*Ͷ�����׼������*/
								break;
			default:break;
		}
}
