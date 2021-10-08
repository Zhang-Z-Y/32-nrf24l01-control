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
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//推挽复用
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
}


/*********************************************************************************
***********************      这一坨都是和电位器有关的          *********************
*********************************************************************************/
//方向键电压检测初始化
void ADC_TurnInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2,ENABLE);//使能
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12
	
	//PA0,3 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	
	//上下用独立的ADC1
	ADC_DeInit(ADC1);//复位ADC1
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器 
	
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
	
	
	//左右用ADC2
	ADC_DeInit(ADC2);//复位ADC2
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC2, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器 
	
	ADC_Cmd(ADC2, ENABLE);	//使能指定的ADC2
	
	ADC_ResetCalibration(ADC2);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC2));	//等待复位校准结束
	
	ADC_StartCalibration(ADC2);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC2));	 //等待校准结束
	
}

//获得上下ADC值
u16 Get_THRAdc(void)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, 0, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

//获得左右 ADC值
u16 Get_ROLLAdc(void)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC2, 3, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC2);	//返回最近一次ADC1规则组的转换结果
}

/*********************************************************************************
***********************      END                             *********************
*********************************************************************************/

u8 txbuf[5];
int main(void)
{
//	int t;
	u16 adc1,adc2;
	//adc1=(u8)Get_ROLLAdc();//左右
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

//	while(W25QXX_ReadID()!=W25Q128);//检测不到W25Q128时,DS0闪烁
//	W25QXX_Write((u8*)txbuf,FLASH_SIZE-80,5);			//从倒数第100个地址处开始,写入SIZE长度的数据
//	W25QXX_Read(datatemp,FLASH_SIZE-80,5);					//从倒数第100个地址处开始,读出SIZE个字节	
	NRF24L01_TX_Mode();
	while(1)
	{
	//	while(NRF24L01_Check())
	//		printf("EOEEOR");  //向串口1发送数据
		adc1=(u16)Get_ROLLAdc();//左右
		adc2=(u16)Get_THRAdc();//上下
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
			delay_ms(50); //如果发送成功，延时50Ms

	//	W25QXX_Write((u8*)txbuf,FLASH_SIZE-100,5);			//从倒数第100个地址处开始,写入SIZE长度的数据
	//	W25QXX_Read(datatemp,FLASH_SIZE-100,5);					//从倒数第100个地址处开始,读出SIZE个字节
		
		printf("上下为:");
		printf("%d\n",Get_THRAdc());
		printf("左右为:");
		printf("%d\n",Get_ROLLAdc());
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		delay_ms(10);
	}
}


