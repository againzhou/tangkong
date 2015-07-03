
#include  <stm32f10x.h>


u32 Fml_Constant=0X8f692ADC; // ���뵽��ʽ�ĳ���
u8 *C= (u8*)&Fml_Constant;//�ѳ���ת��Ϊ����
u16 Fml_CRC16;

/********************************************************************
�������ܣ�CRC16 ����(�˺������ǹ�ʽ,���������һ�ٶ�����ʽ����Щ����ô˺�������
������,ʵ��Ӧ��ʱ�������ѵĳ����м���)
��ڲ�����p Ϊ����ָ�룬len Ϊ���ݳ���
�� �أ�8 λ���
�� ע��
********************************************************************/
u16 Caculate_CRC16(u8 *p,u8 len)//�˺��������������һ�ٶ����ʽ�ڲ����õ�.�û����ϼ���.
{
	u8 i;
	while(len--)
	{
		for(i=0x80; i!=0; i>>=1)
		{
			if((Fml_CRC16 & 0x8000) != 0)
				{
					Fml_CRC16 <<= 1;
					Fml_CRC16 ^= 0x1021;
				}
			else
				{
					Fml_CRC16 <<= 1;
				}
			if((*p&i)!=0)
				{
					Fml_CRC16 ^= 0x1021;
				}
		}
		p++;
	}
	return Fml_CRC16;
}

/********************************************************************
�������ܣ���ʽ
��ڲ�����D
�� ע��
********************************************************************/
void Caculate_120(u8 *D,u8 *Result)
{
u8 *crc;
u16 dat;
crc = (u8*)&dat;
Fml_CRC16 = 0;
dat = Caculate_CRC16(D+6,6);
Result[0] = crc[0];
Result[1] = crc[1];
Result[2] = C[2] ^ D[0] ^ D[1] ^ D[2]^ D[3] ;
Result[3] = C[3] ^ D[4] ^ D[5] ;
}
/// End of file -----------------------------------------------------------------------------------------