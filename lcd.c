#include <8052.h>
#include "descend.h"
#include "lcd.h"

void delay(unsigned int i)
{
	while(i--);
}

void lcd1602_inst_write(unsigned char opr)
{	
	LCD1602_RS=LCD1602_INST;
	LCD1602_DB=opr;
	delay(10);
	LCD1602_EN=1;
	delay(120);
	LCD1602_EN=0;
	delay(10);
}

void lcd1602_data_write(unsigned char target)
{
	LCD1602_RS=LCD1602_DATA;
	LCD1602_DB=target;
	delay(10);
	LCD1602_EN=1;
	delay(120);
	LCD1602_EN=0;
	delay(10);
}

unsigned char old_offset;

void lcd1602_init()
{
	old_offset=0;
	LCD1602_RW=LCD1602_WRITE;
	lcd1602_inst_write(0b00111000);//init
	lcd1602_inst_write(0b00001100);//no cursor
	lcd1602_inst_write(0b00000110);//cursor >> by 1 for after a char
	lcd1602_inst_write(0b00000001);//clear screen
	lcd1602_inst_write(0b10000000);//move cursor, y=0
	lcd1602_data_write(0);
	lcd1602_data_write(1);
	lcd1602_data_write(2);
	lcd1602_data_write(3);
	lcd1602_inst_write(0b11000000);//move cursor, y=1
	lcd1602_data_write(4);
	lcd1602_data_write(5);
	lcd1602_data_write(6);
	lcd1602_data_write(7);
}

void refresh_offset()
{
	if(old_offset<offset_bitmap) lcd1602_inst_write(0b00011100);//screen >>
	else lcd1602_inst_write(0b00011000);//screen <<
	old_offset=offset_bitmap;
}

void display()
{
	/*unsigned char mask;
	mask=0b11111111;
	for(unsigned char i=0;i<8;i++)
	{
		mask<<=1;
		P2=mask;
		delay(1000);
	}
	mask=0b00000001;
	for(unsigned char i=0;i<8;i++)
	{
		
		mask<<=1;
		mask|=0b00000001;
		P2=mask;
		delay(1000);
	}
	for(unsigned char i=0;i<40;i++)
	{
		P2=framebuffer[i];
		delay(10000);
	}*/
	for(unsigned char i=0;i<8;i++)//for custom tile 0-8
	{
		lcd1602_inst_write(0b01000000|(i<<3));
		for(unsigned char j=0;j<8;j++)//for line 0-8 in tile
		{
			unsigned char line=0;
			unsigned char mask=0b00000001<<j;
			for(unsigned char k=0;k<5;k++)
			{
				line<<=1;
				if(framebuffer[i*5+k]&mask) line|=0b00000001;
			}
			lcd1602_data_write(line);
		}
	}
	if(offset_bitmap!=old_offset)
	{
		refresh_offset();
		//old_offset=offset_bitmap;
	}
}

/*void main()
{
	lcd1602_init();
	framebuffer[0]=0b00011111;
	framebuffer[1]=0b00010001;
	framebuffer[2]=0b00010001;
	framebuffer[3]=0b00010001;
	framebuffer[4]=0b00001001;
	framebuffer[5]=0b00000001;
	framebuffer[6]=0b00000010;
	framebuffer[7]=0b00000100;
	framebuffer[8]=0b00001000;
	framebuffer[9]=0b00010000;
	framebuffer[10]=0b00100000;
	framebuffer[11]=0b01000000;
	framebuffer[12]=0b10000000;
	framebuffer[13]=0b01000000;
	framebuffer[14]=0b00100000;
	framebuffer[15]=0b00010000;
	framebuffer[16]=0b00001000;
	framebuffer[17]=0b00000100;
	framebuffer[18]=0b00000010;
	framebuffer[19]=0b00000001;
	framebuffer[20]=0b11111111;
	framebuffer[21]=0b10000010;
	framebuffer[22]=0b10000100;
	framebuffer[23]=0b10001000;
	framebuffer[24]=0b10010000;
	framebuffer[25]=0b10100000;
	framebuffer[26]=0b11000000;
	framebuffer[27]=0b00000000;
	framebuffer[28]=0b00100100;
	framebuffer[29]=0b00100100;
	framebuffer[32]=0b00100100;
	framebuffer[33]=0b00100100;
	framebuffer[39]=0b11110000;
	while(1)
	{
		for(unsigned char i=0;i<=12;i++)
		{
			bitmap_offset=i;
			display();
			delay(5000);
		}
	}
	//lcd1602_inst_write(0b10000000);//move cursor, y=0
	//lcd1602_inst_write(0b11000000);//move cursor, y=1	
	lcd1602_inst_write(0b01000000);
	for(unsigned char i=0;i<8;i++)
	{
		lcd1602_data_write(0b00010101);
	}
	while(1)
	{
		lcd1602_inst_write(0b10000000);//move cursor, y=0
		delay(50000);
		lcd1602_data_write(0);
		delay(50000);
		lcd1602_data_write(1);
		delay(50000);
		lcd1602_data_write(2);
		delay(50000);
		lcd1602_data_write(3);
		delay(50000);
		lcd1602_data_write(4);
		delay(50000);
		lcd1602_data_write('A');
		delay(50000);
		lcd1602_data_write('b');
		delay(50000);
		lcd1602_data_write('Z');
		delay(50000);
		lcd1602_data_write('x');
		delay(50000);
	}
}
*/
