#include <avr/io.h>
#include <avr/delay.h>
#include <util/delay.h>

#include "lcd.h"

//Function to Reset LCD
void lcd_set_4bit()
{
	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x30;				//Sending 3 in the upper nibble
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x30;				//Sending 3 in the upper nibble
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x30;				//Sending 3 in the upper nibble
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x20;				//Sending 2 in the upper nibble to initialize LCD 4-bit mode
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//delay
	cbit(lcd_port,EN);				//Clear Enable Pin
}

//Function to Initialize LCD
void lcd_init()
{
	_delay_ms(1);

	lcd_wr_command(0x28); //4-bit mode and 5x8 dot character font
	lcd_wr_command(0x01); //Clear LCD display
	lcd_wr_command(0x06); //Auto increment cursor position
	lcd_wr_command(0x0E); //Turn on LCD and cursor
	lcd_wr_command(0x80); //Set cursor position
}

	 
//Function to write command on LCD
void lcd_wr_command(unsigned char cmd)
{
	unsigned char temp;
	temp = cmd;
	temp = temp & 0xF0;
	lcd_port &= 0x0F;
	lcd_port |= temp;
	cbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);
	
	cmd = cmd & 0x0F;
	cmd = cmd<<4;
	lcd_port &= 0x0F;
	lcd_port |= cmd;
	cbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);
}

//Function to write data on LCD
void lcd_wr_char(char letter)
{
	char temp;
	temp = letter;
	temp = (temp & 0xF0);
	lcd_port &= 0x0F;
	lcd_port |= temp;
	sbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);

	letter = letter & 0x0F;
	letter = letter<<4;
	lcd_port &= 0x0F;
	lcd_port |= letter;
	sbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);
}



void lcd_home()
{
	lcd_wr_command(0x80);
}

void lcd_clear (void)
{
	lcd_wr_command(0x01);
}
//Function to Print String on LCD
void lcd_string(char *str)
{
	while(*str != '\0')
	{
		lcd_wr_char(*str);
		str++;
	}
}

//Position the LCD cursor at "row", "column"

void lcd_cursor (char row, char column)
{
	switch (row) {
		case 1: lcd_wr_command (0x80 + column - 1); break;
		case 2: lcd_wr_command (0xc0 + column - 1); break;
		case 3: lcd_wr_command (0x94 + column - 1); break;
		case 4: lcd_wr_command (0xd4 + column - 1); break;
		default: break;
	}
}

//Function to print any input value up to the desired digit on LCD
void lcd_print (char row, char coloumn, unsigned int value, int digits)
{
	unsigned char flag=0;
	if(row==0||coloumn==0)
	{
		lcd_home();
	}
	else
	{
		lcd_cursor(row,coloumn);
	}
	if(digits==5 || flag==1)
	{
		million=value/10000+48;
		lcd_wr_char(million);
		flag=1;
	}
	if(digits==4 || flag==1)
	{
		temp = value/1000;
		thousand = temp%10 + 48;
		lcd_wr_char(thousand);
		flag=1;
	}
	if(digits==3 || flag==1)
	{
		temp = value/100;
		hundred = temp%10 + 48;
		lcd_wr_char(hundred);
		flag=1;
	}
	if(digits==2 || flag==1)
	{
		temp = value/10;
		tens = temp%10 + 48;
		lcd_wr_char(tens);
		flag=1;
	}
	if(digits==1 || flag==1)
	{
		unit = value%10 + 48;
		lcd_wr_char(unit);
	}
	if(digits>5)
	{
		lcd_wr_char('E');
	}
}
void lcd_string_rob(char row, char column, char *str)
{
	while(*str != '\0')
	{
		lcd_wr_char_rob(row, column, *str);
		str++;
		column+=1;
	}
}

void lcd_wr_char_rob(char row, char column, char alpha_num_char)
{
	lcd_cursor (row, column);
	
	char temp;
	
	temp = alpha_num_char;
	temp = (temp & 0xF0);
	lcd_data_port_reg &= 0x0F;
	lcd_data_port_reg |= temp;
	
	sbit( lcd_control_port_reg, RS_pin );				// RS = 0 --- Data Input
	cbit( lcd_control_port_reg, RW_pin );				// RW = 0 --- Writing to LCD
	sbit( lcd_control_port_reg, EN_pin );				// Set Enable Pin
	_delay_ms(5);										// Delay
	cbit( lcd_control_port_reg, EN_pin );				// Clear Enable Pin
	
	alpha_num_char = (alpha_num_char & 0x0F);
	alpha_num_char = (alpha_num_char << 4);
	lcd_data_port_reg &= 0x0F;
	lcd_data_port_reg |= alpha_num_char;
	
	sbit( lcd_control_port_reg, RS_pin );				// RS = 0 --- Data Input
	cbit( lcd_control_port_reg, RW_pin );				// RW = 0 --- Writing to LCD
	sbit( lcd_control_port_reg, EN_pin );				// Set Enable Pin
	_delay_ms(5);										// Delay
	cbit( lcd_control_port_reg, EN_pin );				// Clear Enable Pin
}