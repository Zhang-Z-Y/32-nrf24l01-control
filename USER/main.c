#include "stm32f10x.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "w25qxx.h"
#include "NRF.h"

//#define NRF24L01_CSN PBout(12)
//#define NRF24L01_IQR PCout(14)
//#define NRF24L01_CE PBout(5)

void GPIO_PC13_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//���츴��
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
}


/*********************************************************************************
***********************      ��һ�綼�Ǻ͵�λ���йص�          *********************
*********************************************************************************/
//�������ѹ����ʼ��
void ADC_TurnInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2,ENABLE);//ʹ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12
	
	//PA0,3 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	
	//�����ö�����ADC1
	ADC_DeInit(ADC1);//��λADC1
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ��� 
	
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
	
	
	//������ADC2
	ADC_DeInit(ADC2);//��λADC2
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC2, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ��� 
	
	ADC_Cmd(ADC2, ENABLE);	//ʹ��ָ����ADC2
	
	ADC_ResetCalibration(ADC2);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC2));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC2);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC2));	 //�ȴ�У׼����
	
}

//�������ADCֵ
u16 Get_THRAdc(void)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, 0, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}

//������� ADCֵ
u16 Get_ROLLAdc(void)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC2, 3, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC2);	//�������һ��ADC1�������ת�����
}

/*********************************************************************************
***********************      END                             *********************
*********************************************************************************/

u8 txbuf[5];
int main(void)
{
//	int t;
	u16 adc1,adc2;
	//adc1=(u8)Get_ROLLAdc();//����
	u8 tmp_buf1[5];
	u8 tmp_buf2[5];
	u8 A[5]="123";
//	u8 datatemp[5],FLASH_SIZE=5;
//	txbuf[0]=4;
	uart_init(115200);
	GPIO_PC13_Init();
	ADC_TurnInit();
	delay_init();
	NRF24L01_Init();

//	while(W25QXX_ReadID()!=W25Q128);//��ⲻ��W25Q128ʱ,DS0��˸
//	W25QXX_Write((u8*)txbuf,FLASH_SIZE-80,5);			//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
//	W25QXX_Read(datatemp,FLASH_SIZE-80,5);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�	
	NRF24L01_TX_Mode();
	while(1)
	{
	//	while(NRF24L01_Check())
	//		printf("EOEEOR");  //�򴮿�1��������
		adc1=(u16)Get_ROLLAdc();//����
		adc2=(u16)Get_THRAdc();//����
		tmp_buf1[0]=(adc1/1000)+'0';
		tmp_buf1[1]=(adc1/100%10)+'0';
		tmp_buf1[2]=(adc1%100/10)+'0';
		tmp_buf1[3]=(adc1%10)+'0';
		tmp_buf1[4]=' ';
		
		tmp_buf2[0]=(adc2/1000)+'0';
		tmp_buf2[1]=(adc2/100%10)+'0';
		tmp_buf2[2]=(adc2%100/10)+'0';
		tmp_buf2[3]=(adc2%10)+'0';
		tmp_buf2[4]=' ';
//		if((0<adc1)&&(1040>adc1))
//			txbuf[1]=1;
//		else txbuf[1]=0;
//		if((1041<adc1)&&(2100>adc1))
//			txbuf[2]=1;
//		else txbuf[2]=0;
//		
//		if((0<adc2)&&(1100>adc2))
//			txbuf[3]=1;
//		else txbuf[3]=0;
//		if((1101<adc2)&&(1300>adc2))
//			txbuf[4]=1;
//		else txbuf[4]=0;
//		if((1301<adc2)&&(2050>adc2))
//		  txbuf[4]=2;
//		else txbuf[4]=0;
//	    tmp_buf[={adc1,0,0,0,0};
		//txbuf[5]=(u8)tmp_buf[5];
		if(NRF24L01_TxPacket(tmp_buf2)==TX_OK);
			printf("adc2OK");
		if(NRF24L01_TxPacket(tmp_buf1)==TX_OK);
			printf("adc1OK");
		//NRF24L01_TxPacket(tmp_buf1);
//		if(NRF24L01_TxPacket(txbuf)==TX_OK)
			delay_ms(50); //������ͳɹ�����ʱ50Ms

	//	W25QXX_Write((u8*)txbuf,FLASH_SIZE-100,5);			//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
	//	W25QXX_Read(datatemp,FLASH_SIZE-100,5);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
		
		printf("����Ϊ:");
		printf("%d\n",Get_THRAdc());
		printf("����Ϊ:");
		printf("%d\n",Get_ROLLAdc());
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		delay_ms(10);
	}
}


