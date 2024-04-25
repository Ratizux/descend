#ifndef LCD_H
#define LCD_H

void delay(unsigned int i);

#define LCD1602_DB P0
#define LCD1602_EN P2_7
#define LCD1602_RW P2_5
#define LCD1602_READ 1
#define LCD1602_WRITE 0
#define LCD1602_RS P2_6
#define LCD1602_INST 0
#define LCD1602_DATA 1

void lcd1602_inst_write(unsigned char opr);

void lcd1602_data_write(unsigned char target);

void lcd1602_init();

void refresh_offset();

void display();
#endif
