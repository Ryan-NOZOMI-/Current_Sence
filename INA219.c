/* INA219 */
/* ���{���ΨӴ���  �̰򥻪�Ū���q�y��ܩ�C�q��ܾ��W �����ܹq���P�q�y (DELAY25us�O���ϥ�24MHZ�����_) */
/* �C�q��ܱ��y���ΤFPort1 , Port0   �ҥH���n�ϥ�*/
#include <AT89S53.H>
#define scan_speed 2304					//�C�q���y�t�� �N1ms �]���@���ɶ���� 24: 12 , 12:6
#define TH_M1 (65536-scan_speed)/256;	//�C�q���y�t��
#define TL_M1 (65536-scan_speed)%256;	//�C�q���y�t��

sbit KEY_SW1=P2^0;					//���s1 ++
sbit KEY_SW2=P2^1;					//���s2 --
sbit KEY_SW3=P2^2;					//���s3 SHOW MEM
sbit KEY_SW4=P2^4;					//���s4 N/A
sbit KEY_SW5=P2^3;					//���s5	N/A
sbit KEY_SW6=P2^7;					//���s6	WRITE MEM

sbit seg_1=P1^0;					//�C�q��� - C1
sbit seg_2=P1^1;					//�C�q��� - C2
sbit seg_3=P1^2;					//�C�q��� - C3
sbit seg_4=P1^3;					//�C�q��� - C4
									//	0  ,  1 ,  2 ,  3 ,  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  A ,  b ,  C ,  d ,  E ,  F , _h ,  - ,  _ .    .
									//	0  ,  1 ,  2 ,  3 ,  4 ,  5 ,  6 ,  7 ,  8 ,  9 , 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 .
unsigned char code Seg_Decode_Table[]={0x03,0x9f,0x25,0x0d,0x99,0x49,0x41,0x1b,0x01,0x19,0x11,0xC1,0x63,0x85,0x61,0x71,0x7F,0xFD,0xEF,0xFF};         //�C�q��ܾ� �ѽX������  �@�����C�q��ܽX0-F
unsigned char code Seg_Scan_Port[]={0xFE,0xFD,0xFB,0xF7};			//�C�q���yINDEX	�tCODE��ܩw�q�b�{����[�������O��Ū]
unsigned char Seg_Digit[4] = {17,17,17,17};			//�C�q��� �|�ƭȤ���Ȧs [���y�ݭn] �w�]"----"

unsigned char Seg_Scan_Index = 0;
unsigned char i;  //�j����ܼ�
//-----------------------------
bit change_disp = 1;
//-----------------------------

sbit SDA=P3^0;						//I2C SDA
sbit SCL=P3^1;						//I2C SCL

unsigned int INA219_Read_Word(unsigned char Pointer_Address);	//1WordŪ�� [RANDOM READ]
void INA219_Write_Word(unsigned char Pointer_Address,unsigned int Write_Word);	//2Byte�g�J 1 Word
//I2C Control
void sendbit(bit);					//I2C�ǰe1bit
void sendbyte(char);				//I2C�ǰe1Byte = 8bit (char)
bit readbit(void);					//I2C����1bit
unsigned char readbyte(void);		//I2C����1Byte = 8bit (char)
void send_start_signal(void);		//I2C�_�l�H��
void send_stop_signal(void);		//I2C�����H��
void check_ACK_bit(void);			//I2C����^��ACK
void delay25us(unsigned int);		//���ɫ��O

//START==================�D�{��===========
main()
{
	unsigned int Value_SV,Value_V,Value_I,Value_P;
	SCL = 1;
	SDA = 1;
	IE = 0x82;			//�Ұ�Interrupt TIME 0
	TMOD = 0xF1;		//TIME 0 �ϥ�16�줸�p�ɭp�ƾ�
	TH0=TH_M1;TL0=TL_M1;//�]�w��l��
	TR0=1;				//�Ұ�TIMER 0	�C�q��ܱ��y��
	delay25us(1000);

	while(1)				//���ݫ��s�Q���U���L���j��
	{
		if(KEY_SW1==0)
		{
			unsigned int Temp;

			P1_7 = 0;		//�G
			INA219_Write_Word(0x00,0x019F);
			delay25us(3000);
			Value_SV = INA219_Read_Word(0x01);		//Shunt Voltage
			Value_V = INA219_Read_Word(0x02);		//Bus Voltage
			INA219_Write_Word(0x05,0x5000);
			Value_I  = INA219_Read_Word(0x04);		//Current 
			Value_P  = INA219_Read_Word(0x03);		//Power
			delay25us(3000);
			P1_7 = 1;		//���G

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
}//END==================�D�{������===========


//START==============�C�q��ܱ��y�p�ɤ��_===========
void timer0(void) interrupt 1
{
	TH0=TH_M1;TL0=TL_M1;
	P0=0xFF;					//������ܸ�ƽu
	switch (Seg_Scan_Index)		//��ƿ��
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
	P0=Seg_Decode_Table[Seg_Digit[Seg_Scan_Index]];				//��ܧ��� ��ƽu��Ʃ��P0
	if(++Seg_Scan_Index==4) Seg_Scan_Index=0;					//�|�ӱ��y���F  �^��Ĥ@�Ӧ��
}//END==============�C�q��ܱ��y�p�ɤ��_����===========


//===================================24C02 CONTROL FUNCTION===================================
void INA219_Write_Word(unsigned char Pointer_Address,unsigned int Write_Word)	//�g�J 1 Word
{
	send_start_signal();
	//���ɥ��n�I�s�Ӹ˸mID ��Ū�g���A [0-W] [1-R]
	sendbyte(0x80);				//�g�J (����)A1/A0��GND
	sendbyte(Pointer_Address);	//ADDRESS �a�}
	//-��Ƽg�J�i�}�l-
	sendbyte((unsigned char)(Write_Word >> 8));
	sendbyte((unsigned char)Write_Word & 0x00FF);
	//-��Ƽg�J����-
	send_stop_signal();
}//=======================1Byte�g�J����=======================


unsigned int INA219_Read_Word(unsigned char Pointer_Address)					//Ū��1Word [RANDOM READ]
{
	unsigned int temp;
	unsigned char t1,t2;
	send_start_signal();
	//���ɬ�RANDOM READ	���ϥμg�J���A+�a�}  �ӧ��� �O����s����m����
	sendbyte(0x80);					//Ū�� (����)A1/A0��GND
	sendbyte(Pointer_Address);		//ADDRESS (WORD)�a�}
	send_start_signal();			//���жǰe�_�l�H��
	//��m���Фw���w  �ǰe��Ū���� BLOCK��m ��Ū�g���A [0-W] [1-R]
	sendbyte(0x81);					//Ū�� (����)A1/A0��GND
	//-���Ū���i�}�l- 
	t1 = readbyte();
	t2 = readbyte();
	temp = (t1<<8)|t2;
	//-���Ū������-
	send_stop_signal();
	return temp;
}//=======================1ByteŪ������=======================
//===================================24C02 CONTROL FUNCTION===================================

//================================================I2C CONTROL FUNCTION=================================================================
void send_start_signal(void)
{
	//�H�U��START SINGEL
	SDA = 1;
	delay25us(1);
	SCL = 1;
	delay25us(1);
	SDA = 0;		//SCL��HIGH��   ��ƽu�q1��0  �����}�l�T��
	delay25us(1);
}	//�H�W��START SINGEL


void send_stop_signal(void)
{
	//�H�U��STOP SINGEL
	SCL = 0;		//���ɸ�ƵL��
	delay25us(1);
	SDA = 0;		//�n�쥿�t �����U��0
	delay25us(1);
	SCL = 1;		//SCL HIGH
	delay25us(1);	
	SDA = 1;		// SCL��HIGH��  ��ƽu�q0��1 ���������T��
}	//�H�W��STOP SINGEL


void sendbyte(unsigned char thebyte)
{	//�H�U���ǰe1 BYTE (8bits)
	char i;
	bit bit_temp;
	for(i=0;i<8;i++)
	{
		bit_temp = (thebyte >> (7-i))& 0x01; //�N8bits �����@�ӭ�bit�e�X MSB���o�e
		sendbit(bit_temp);
	}
	check_ACK_bit();	//��Ʊ����T�{�^��
}	//�H�W���ǰe1 BYTE (8bits)


void sendbit(bit SDA_BIT)
{	//�@bit�ǰe
	SCL = 0;
	delay25us(1);	//DATA HOLD �W�ɮɶ�
	SDA = SDA_BIT;	//�}�l�ǰe ��bit
	delay25us(1);	//DATA HOLD �W�ɮɶ�
	SCL = 1; 		//SCL = HIGH (1)  Data Valid
	delay25us(2);	//DATA HOLD ��Ƨ��
	SCL = 0; 		//SCL = LOW  (0) ��Ƥw�ǤJ SDA�i��
	delay25us(1);	//DATA HOLD	�U���ɶ�
}	//�H�W �@bit�ǰe����


bit readbit(void)
{	//�@bit����
	bit temp;
	delay25us(1);	//DATA HOLD ����SDA�ܤƮɶ�
	SCL = 1;	 	//SCL = HIGH (1)  Data Valid
	delay25us(1);	//DATA HOLD ��Ƨ������
	temp = SDA;		//����	��Ƨ����TEMP
	delay25us(1);	//DATA HOLD ��Ƨ������
	SCL = 0; 		//SCL = LOW  (0) ��Ƥw�ǤJ SDA�i��
	delay25us(1);	//DATA HOLD	�U���ɶ�
	return temp;
}	//�H�W �@bit��������


unsigned char readbyte(void)
{	//�H�U������1 BYTE (8bits)
	unsigned char read_byte = 0x00;
	SDA = 1;		//����SDA LINE
	for(i=0;i<8;i++)
	{
		read_byte = read_byte << 1;							//�`�첾7��  �Ĥ@���Ŧ첾�ʧ@
		read_byte = read_byte | (unsigned char)readbit(); 	//�N8bits �����@�ӭ�bit�e�X MSB������
	}
	SCL = 0;
	delay25us(1);	//DATA HOLD �W�ɮɶ�
	SDA = 0;		//�}�l�ǰe ��bit
	delay25us(1);	//DATA HOLD �W�ɮɶ�
	SCL = 1; 		//SCL = HIGH (1)  Data Valid
	delay25us(2);	//DATA HOLD ��Ƨ��
	SCL = 0; 		//SCL = LOW  (0) ��Ƥw�ǤJ SDA�i��
	delay25us(1);	//DATA HOLD	�U���ɶ�
	return read_byte;
}	//�H�W������1 BYTE (8bits)


void check_ACK_bit(void)
{	//�H�U����Ʊ�������  �^�Ǥ�ACK�H�����T�{
	SDA = 1;		//���ɭn����ACK�H�� (��9�줸)  SLAVE�b����8�Ӧ줸�� ���\�|�Ǧ^0 �ҥH���NMASTER����X��HIGH
	delay25us(1);
	SCL = 1; 		//SCL = HIGH (1)  Data Valid  ��9��CLK
	while(SDA){P1_6=0; delay25us(5000);P1_6=1;delay25us(5000);}		//��^�ǫH�����O�C�A[���NAK] ���ɤ��~��u�@  ��LED�{�{  ==> ����SDA�C�A (�����������߮ɤ~���L)
	delay25us(2);	// ���\�����H�� CLK�~��
	SCL = 0; 		//SCL = LOW  (0) ��Ƥw�ǤJ SDA�i��
	delay25us(1);
}	//�H�W����Ʊ�������  �^�Ǥ�ACK�H�����T�{
//================================================I2C CONTROL FUNCTION=================================================================


//����25us�Ƶ{��
void delay25us(unsigned int x)
{
	unsigned int i;
    char j;
	for(i=0;i<x;i++)
		for(j=0;j<6;j++);
}//����25us�Ƶ{������