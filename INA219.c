/* INA219 */
/* 此程式用來測試  最基本的讀取電流顯示於七段顯示器上 交替顯示電壓與電流 (DELAY25us是指使用24MHZ的晶震) */
/* 七段顯示掃描佔用了Port1 , Port0   所以不要使用*/
#include <AT89S53.H>
#define scan_speed 2304					//七段掃描速度 將1ms 設為一次時間基準 24: 12 , 12:6
#define TH_M1 (65536-scan_speed)/256;	//七段掃描速度
#define TL_M1 (65536-scan_speed)%256;	//七段掃描速度

sbit KEY_SW1=P2^0;					//按鈕1 ++
sbit KEY_SW2=P2^1;					//按鈕2 --
sbit KEY_SW3=P2^2;					//按鈕3 SHOW MEM
sbit KEY_SW4=P2^4;					//按鈕4 N/A
sbit KEY_SW5=P2^3;					//按鈕5	N/A
sbit KEY_SW6=P2^7;					//按鈕6	WRITE MEM

sbit seg_1=P1^0;					//七段顯示 - C1
sbit seg_2=P1^1;					//七段顯示 - C2
sbit seg_3=P1^2;					//七段顯示 - C3
sbit seg_4=P1^3;					//七段顯示 - C4
									//	0  ,  1 ,  2 ,  3 ,  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  A ,  b ,  C ,  d ,  E ,  F , _h ,  - ,  _ .    .
									//	0  ,  1 ,  2 ,  3 ,  4 ,  5 ,  6 ,  7 ,  8 ,  9 , 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 .
unsigned char code Seg_Decode_Table[]={0x03,0x9f,0x25,0x0d,0x99,0x49,0x41,0x1b,0x01,0x19,0x11,0xC1,0x63,0x85,0x61,0x71,0x7F,0xFD,0xEF,0xFF};         //七段顯示器 解碼對應表  共陽極七段顯示碼0-F
unsigned char code Seg_Scan_Port[]={0xFE,0xFD,0xFB,0xF7};			//七段掃描INDEX	含CODE表示定義在程式區[不能改但是能讀]
unsigned char Seg_Digit[4] = {17,17,17,17};			//七段顯示 四數值分位暫存 [掃描需要] 預設"----"

unsigned char Seg_Scan_Index = 0;
unsigned char i;  //迴圈用變數
//-----------------------------
bit change_disp = 1;
//-----------------------------

sbit SDA=P3^0;						//I2C SDA
sbit SCL=P3^1;						//I2C SCL

unsigned int INA219_Read_Word(unsigned char Pointer_Address);	//1Word讀取 [RANDOM READ]
void INA219_Write_Word(unsigned char Pointer_Address,unsigned int Write_Word);	//2Byte寫入 1 Word
//I2C Control
void sendbit(bit);					//I2C傳送1bit
void sendbyte(char);				//I2C傳送1Byte = 8bit (char)
bit readbit(void);					//I2C接收1bit
unsigned char readbyte(void);		//I2C接收1Byte = 8bit (char)
void send_start_signal(void);		//I2C起始信號
void send_stop_signal(void);		//I2C結束信號
void check_ACK_bit(void);			//I2C檢驗回傳ACK
void delay25us(unsigned int);		//延時指令

//START==================主程式===========
main()
{
	unsigned int Value_SV,Value_V,Value_I,Value_P;
	SCL = 1;
	SDA = 1;
	IE = 0x82;			//啟動Interrupt TIME 0
	TMOD = 0xF1;		//TIME 0 使用16位元計時計數器
	TH0=TH_M1;TL0=TL_M1;//設定初始值
	TR0=1;				//啟動TIMER 0	七段顯示掃描用
	delay25us(1000);

	while(1)				//等待按鈕被按下之無限迴圈
	{
		if(KEY_SW1==0)
		{
			unsigned int Temp;

			P1_7 = 0;		//亮
			INA219_Write_Word(0x00,0x019F);
			delay25us(3000);
			Value_SV = INA219_Read_Word(0x01);		//Shunt Voltage
			Value_V = INA219_Read_Word(0x02);		//Bus Voltage
			INA219_Write_Word(0x05,0x5000);
			Value_I  = INA219_Read_Word(0x04);		//Current 
			Value_P  = INA219_Read_Word(0x03);		//Power
			delay25us(3000);
			P1_7 = 1;		//不亮

			Temp = Value_V>>3;
			Temp <<= 2;
			Seg_Digit[0]	= (Temp /1000)%10;
			Seg_Digit[1]	= (Temp /100)%10;
			Seg_Digit[2]	= (Temp /10)%10;
			Seg_Digit[3]	= Temp%10;

		}
		if(KEY_SW2==0)			//Bus Voltage Display
		{				
			//Temp = Value_V;

			/*
			Seg_Digit[0]	= (Temp & 0xf000)>>12;
			Seg_Digit[1]	= (Temp & 0x0f00)>>8;
			Seg_Digit[2]	= (Temp & 0x00f0)>>4;
			Seg_Digit[3]	= Temp & 0x000f;
			*/			
		}
		if(KEY_SW3==0)
		{
			
		}
		if(KEY_SW4==0)
		{
			
		}
		if(KEY_SW5==0)
		{
			
		}
	}
}//END==================主程式結束===========


//START==============七段顯示掃描計時中斷===========
void timer0(void) interrupt 1
{
	TH0=TH_M1;TL0=TL_M1;
	P0=0xFF;					//關閉顯示資料線
	switch (Seg_Scan_Index)		//位數選擇
	{
		case 0:
			seg_1 = 0; seg_2 = 1; seg_3 = 1; seg_4 = 1; break;
		case 1:                                         
			seg_1 = 1; seg_2 = 0; seg_3 = 1; seg_4 = 1; break;
		case 2:                                         	
			seg_1 = 1; seg_2 = 1; seg_3 = 0; seg_4 = 1; break;	
		case 3:                                             	
			seg_1 = 1; seg_2 = 1; seg_3 = 1; seg_4 = 0; break;                              
	} 
	P0=Seg_Decode_Table[Seg_Digit[Seg_Scan_Index]];				//選擇完畢 資料線資料放至P0
	if(++Seg_Scan_Index==4) Seg_Scan_Index=0;					//四個掃描完了  回到第一個位數
}//END==============七段顯示掃描計時中斷結束===========


//===================================24C02 CONTROL FUNCTION===================================
void INA219_Write_Word(unsigned char Pointer_Address,unsigned int Write_Word)	//寫入 1 Word
{
	send_start_signal();
	//此時先要呼叫該裝置ID 及讀寫狀態 [0-W] [1-R]
	sendbyte(0x80);				//寫入 (此時)A1/A0接GND
	sendbyte(Pointer_Address);	//ADDRESS 地址
	//-資料寫入可開始-
	sendbyte((unsigned char)(Write_Word >> 8));
	sendbyte((unsigned char)Write_Word & 0x00FF);
	//-資料寫入完畢-
	send_stop_signal();
}//=======================1Byte寫入完畢=======================


unsigned int INA219_Read_Word(unsigned char Pointer_Address)					//讀取1Word [RANDOM READ]
{
	unsigned int temp;
	unsigned char t1,t2;
	send_start_signal();
	//此時為RANDOM READ	先使用寫入狀態+地址  來改變 記憶體存取位置指標
	sendbyte(0x80);					//讀取 (此時)A1/A0接GND
	sendbyte(Pointer_Address);		//ADDRESS (WORD)地址
	send_start_signal();			//重覆傳送起始信號
	//位置指標已指定  傳送該讀取之 BLOCK位置 及讀寫狀態 [0-W] [1-R]
	sendbyte(0x81);					//讀取 (此時)A1/A0接GND
	//-資料讀取可開始- 
	t1 = readbyte();
	t2 = readbyte();
	temp = (t1<<8)|t2;
	//-資料讀取完畢-
	send_stop_signal();
	return temp;
}//=======================1Byte讀取完畢=======================
//===================================24C02 CONTROL FUNCTION===================================

//================================================I2C CONTROL FUNCTION=================================================================
void send_start_signal(void)
{
	//以下為START SINGEL
	SDA = 1;
	delay25us(1);
	SCL = 1;
	delay25us(1);
	SDA = 0;		//SCL為HIGH時   資料線從1跳0  此為開始訊號
	delay25us(1);
}	//以上為START SINGEL


void send_stop_signal(void)
{
	//以下為STOP SINGEL
	SCL = 0;		//此時資料無效
	delay25us(1);
	SDA = 0;		//要抓正緣 先跳下成0
	delay25us(1);
	SCL = 1;		//SCL HIGH
	delay25us(1);	
	SDA = 1;		// SCL為HIGH時  資料線從0跳1 此為結束訊號
}	//以上為STOP SINGEL


void sendbyte(unsigned char thebyte)
{	//以下為傳送1 BYTE (8bits)
	char i;
	bit bit_temp;
	for(i=0;i<8;i++)
	{
		bit_temp = (thebyte >> (7-i))& 0x01; //將8bits 分成一個個bit送出 MSB先發送
		sendbit(bit_temp);
	}
	check_ACK_bit();	//資料接收確認回傳
}	//以上為傳送1 BYTE (8bits)


void sendbit(bit SDA_BIT)
{	//一bit傳送
	SCL = 0;
	delay25us(1);	//DATA HOLD 上升時間
	SDA = SDA_BIT;	//開始傳送 該bit
	delay25us(1);	//DATA HOLD 上升時間
	SCL = 1; 		//SCL = HIGH (1)  Data Valid
	delay25us(2);	//DATA HOLD 資料抓取
	SCL = 0; 		//SCL = LOW  (0) 資料已傳入 SDA可變
	delay25us(1);	//DATA HOLD	下降時間
}	//以上 一bit傳送完畢


bit readbit(void)
{	//一bit接收
	bit temp;
	delay25us(1);	//DATA HOLD 等待SDA變化時間
	SCL = 1;	 	//SCL = HIGH (1)  Data Valid
	delay25us(1);	//DATA HOLD 資料抓取延遲
	temp = SDA;		//正式	資料抓取至TEMP
	delay25us(1);	//DATA HOLD 資料抓取延遲
	SCL = 0; 		//SCL = LOW  (0) 資料已傳入 SDA可變
	delay25us(1);	//DATA HOLD	下降時間
	return temp;
}	//以上 一bit接收完畢


unsigned char readbyte(void)
{	//以下為接收1 BYTE (8bits)
	unsigned char read_byte = 0x00;
	SDA = 1;		//釋放SDA LINE
	for(i=0;i<8;i++)
	{
		read_byte = read_byte << 1;							//總位移7次  第一次空位移動作
		read_byte = read_byte | (unsigned char)readbit(); 	//將8bits 分成一個個bit送出 MSB先接收
	}
	SCL = 0;
	delay25us(1);	//DATA HOLD 上升時間
	SDA = 0;		//開始傳送 該bit
	delay25us(1);	//DATA HOLD 上升時間
	SCL = 1; 		//SCL = HIGH (1)  Data Valid
	delay25us(2);	//DATA HOLD 資料抓取
	SCL = 0; 		//SCL = LOW  (0) 資料已傳入 SDA可變
	delay25us(1);	//DATA HOLD	下降時間
	return read_byte;
}	//以上為接收1 BYTE (8bits)


void check_ACK_bit(void)
{	//以下為資料接收完畢  回傳之ACK信號的確認
	SDA = 1;		//此時要接收ACK信號 (第9位元)  SLAVE在接受8個位元後 成功會傳回0 所以先將MASTER的輸出為HIGH
	delay25us(1);
	SCL = 1; 		//SCL = HIGH (1)  Data Valid  第9個CLK
	while(SDA){P1_6=0; delay25us(5000);P1_6=1;delay25us(5000);}		//當回傳信號不是低態[表示NAK] 此時不繼續工作  而LED閃爍  ==> 直到SDA低態 (此等式不成立時才跳過)
	delay25us(2);	// 成功接收信號 CLK繼續
	SCL = 0; 		//SCL = LOW  (0) 資料已傳入 SDA可變
	delay25us(1);
}	//以上為資料接收完畢  回傳之ACK信號的確認
//================================================I2C CONTROL FUNCTION=================================================================


//延遲25us副程式
void delay25us(unsigned int x)
{
	unsigned int i;
    char j;
	for(i=0;i<x;i++)
		for(j=0;j<6;j++);
}//延遲25us副程式結束