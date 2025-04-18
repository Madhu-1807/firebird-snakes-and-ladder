#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include <math.h> //included to support power function
#include "firebird_simulation.h"//included for robo dance
#include "lcd.h"
#include "lcd.c"
#define SEED 8

void port_init();
void timer5_init();
void velocity(unsigned char, unsigned char);
void motors_delay();


unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char Left_white_line = 0;
unsigned char Center_white_line = 0;
unsigned char Right_white_line = 0;

void interrupt_switch_config (void)
{
	DDRE = DDRE & 0x7F;  //PORTE 7 pin set as input
	PORTE = PORTE | 0x80; //PORTE7 internal pull-up enabled
}
bool interrupt_switch_pressed(void)
{
	if (!((PINE & 0x80) == 0x80)) //switch is pressed
	{
		return true;
	}
	else
	{
		return false;
	}
}

void buzzer_pin_config(void) {
	
	DDRC |= ( ( 1 << 3 ) );
	
	PORTC &= ~( ( 1 << 3) );
}


void buzzer_on(void) {
	
	PORTC |= ( ( 1 << 3 ) );
}



void buzzer_off(void) {
	
	PORTC &= ~( ( 1 << 3 ) );
}

//Function to configure LCD port
void lcd_port_config (void)
{
 DDRC = DDRC | 0xF7; //all the LCD pin's direction set as output
 PORTC = PORTC & 0x80; // all the LCD pins are set to logic 0 except PORTC 7
}

//ADC pin configuration
void adc_pin_config (void)
{
 DDRF = 0x00; 
 PORTF = 0x00;
 DDRK = 0x00;
 PORTK = 0x00;
}

//Function to configure ports to enable robot's motion
void motion_pin_config (void) 
{
 DDRA = DDRA | 0x0F;
 PORTA = PORTA & 0xF0;
 DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
 PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

//Function to Initialize PORTS
void port_init()
{
	lcd_port_config();
	adc_pin_config();
	motion_pin_config();	
}

// Timer 5 initialized in PWM mode for velocity control
// Prescale:256
// PWM 8bit fast, TOP=0x00FF
// Timer Frequency:225.000Hz
void timer5_init()
{
	TCCR5B = 0x00;	//Stop
	TCNT5H = 0xFF;	//Counter higher 8-bit value to which OCR5xH value is compared with
	TCNT5L = 0x01;	//Counter lower 8-bit value to which OCR5xH value is compared with
	OCR5AH = 0x00;	//Output compare register high value for Left Motor
	OCR5AL = 0xFF;	//Output compare register low value for Left Motor
	OCR5BH = 0x00;	//Output compare register high value for Right Motor
	OCR5BL = 0xFF;	//Output compare register low value for Right Motor
	OCR5CH = 0x00;	//Output compare register high value for Motor C1
	OCR5CL = 0xFF;	//Output compare register low value for Motor C1
	TCCR5A = 0xA9;	/*{COM5A1=1, COM5A0=0; COM5B1=1, COM5B0=0; COM5C1=1 COM5C0=0}
 					  For Overriding normal port functionality to OCRnA outputs.
				  	  {WGM51=0, WGM50=1} Along With WGM52 in TCCR5B for Selecting FAST PWM 8-bit Mode*/
	
	TCCR5B = 0x0B;	//WGM12=1; CS12=0, CS11=1, CS10=1 (Prescaler=64)
}

void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;		//MUX5 = 0
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//Function For ADC Conversion
unsigned char ADC_Conversion(unsigned char Ch) 
{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;  			
	ADMUX= 0x20| Ch;	   		
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}

//Function To Print Sesor Values At Desired Row And Coloumn Location on LCD
void print_sensor(char row, char coloumn,unsigned char channel)
{
	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, coloumn, ADC_Value, 3);
}

//Function for velocity control
void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}

//Function used for setting motor's direction
void motion_set (unsigned char Direction)
{
 unsigned char PortARestore = 0;

 Direction &= 0x0F; 		// removing upper nibbel for the protection
 PortARestore = PORTA; 		// reading the PORTA original status
 PortARestore &= 0xF0; 		// making lower direction nibbel to 0
 PortARestore |= Direction; // adding lower nibbel for forward command and restoring the PORTA status
 PORTA = PortARestore; 		// executing the command
}

void forward (void) 
{
  motion_set (0x06);
}

void left (void)
{
	motion_set(0x05);
}

void right (void)
{
	motion_set(0x0A);
}

void stop (void)
{
  motion_set (0x00);
}

void init_devices (void)
{
 	cli(); //Clears the global interrupts
	port_init();
	adc_init();
	timer5_init();
	sei();   //Enables the global interrupts
}

int roll_die ()
{
	int die_val = 0;
	die_val=rand()%6 + 1;
	return die_val;
}
void static_loop() 
{
	lcd_string_rob(1, 1, "Reached Home!");
	lcd_string_rob(2, 1, "Congratulations!!");
}
void static_loop2()
{
	lcd_string_rob(2,1,"Game Over :) ");
}
void bar_graph_led_pins_config(void) 
{
	// << NOTE >> : Use Masking and Shift Operators here
	
	// Make all LED pins as output
	bar_graph_led_ddr_reg |= 0xFF;
}

void turn_off_bar_graph_led(unsigned char led_pin) {
	// << NOTE >> : Use Masking and Shift Operators here
	
	// Set ONLY a particular Bar-graph LED pin as low (OFF)
	bar_graph_led_port_reg	&= ~( ( 1 << led_pin ) );
}

void turn_on_bar_graph_led(unsigned char led_pin) {
	// << NOTE >> : Use Masking and Shift Operators here
	
	// Set ONLY a particular Bar-graph LED pin as high (ON)
	bar_graph_led_port_reg	|= ( ( 1 << led_pin ) );
}

//Main Function
int main()
{
	int final_pos = 0;
	int node = 0;
	init_devices();
	lcd_set_4bit();
	lcd_init();
	srand(SEED); //generates new set of random numbers
	while(1)
	{
		
		while(final_pos == 0)
		{
  		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
  		Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
  		Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
  		flag=0;

  		print_sensor(1,1,3);	//Prints value of White Line Sensor1
  		print_sensor(1,5,2);	//Prints Value of White Line Sensor2
  		print_sensor(1,9,1);	//Prints Value of White Line Sensor3
  		
  		if (interrupt_switch_pressed() == true)
  		{
	  		int die_num = 0;
	  		die_num = roll_die();
	  		final_pos = final_pos + die_num;
	  		lcd_print(2, 4, die_num, 2);
			lcd_print(2, 7, final_pos, 2);
  		}	  
		stop();
		}
		while(final_pos != 0)
		{
					Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
					Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
					Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
					flag=0;

					print_sensor(1,1,3);	//Prints value of White Line Sensor1
					print_sensor(1,5,2);	//Prints Value of White Line Sensor2
					print_sensor(1,9,1);	//Prints Value of White Line Sensor3
					
					if (interrupt_switch_pressed() == true)
					{
						int die_num = 0;
						die_num = roll_die();
						final_pos = final_pos + die_num;
						lcd_print(2, 4, die_num, 2);
						lcd_print(2, 7, final_pos, 2);
					}

					while (final_pos > 26)
					{
						final_pos = 26;
						lcd_print(2, 7, final_pos, 2);
					}
					while ((final_pos == 3) && (node == 3))
					{
						buzzer_on();
						_delay_ms(100);
						buzzer_off();
						_delay_ms(50);
						buzzer_on();
						_delay_ms(100);
						buzzer_off();
						_delay_ms(50);
						buzzer_on();
						_delay_ms(100);
						buzzer_off();
						_delay_ms(50);
						
						forward();
						_delay_ms(400);
						left();
						_delay_ms(900);
						stop();
						final_pos = 100;
					}
					while (final_pos == 100)
						{
							Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
							Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
							Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
							flag=0;

							print_sensor(1,1,3);	//Prints value of White Line Sensor1
							print_sensor(1,5,2);	//Prints Value of White Line Sensor2
							print_sensor(1,9,1);	//Prints Value of White Line Sensor3
							
							lcd_print(2, 7, final_pos, 2);
							if(Center_white_line>0x07)
							{
								flag=1;
								forward();
								velocity(160,160);
							}

							if((Left_white_line<0x07) && (flag==0))
							{
								flag=1;
								forward();
								velocity(130,50);
							}

							if((Right_white_line<=0x07) && (flag==0))
							{
								flag=1;
								forward();
								velocity(50,130);
							}

							if(Center_white_line<0x07 && Left_white_line<0x07 && Right_white_line<0x07)
							{
								forward();
								velocity(160,160);
								_delay_ms(500);
							}
							
							if(Center_white_line>0x07 && Left_white_line>0x07 && Right_white_line>0x07)
							{
								final_pos = 17;;
								lcd_print(2, 7, final_pos, 2);
								node = 17;
								lcd_print(2, 1, node, 2);
								
								buzzer_on();
								_delay_ms(100);
								buzzer_off();
								_delay_ms(50);
								
								forward();
								velocity(150,150);
								_delay_ms(700);
								left();
								_delay_ms(1900);
							}
					  }
					  while ((final_pos == 16) && (node == 16))
					  {
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  
						  left();
						  _delay_ms(1500);
						  stop();
						  final_pos = 101;
					  }
					  while (final_pos == 101)
					  {
						  Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
						  Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
						  Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
						  flag=0;

						  print_sensor(1,1,3);	//Prints value of White Line Sensor1
						  print_sensor(1,5,2);	//Prints Value of White Line Sensor2
						  print_sensor(1,9,1);	//Prints Value of White Line Sensor3
						  
						  lcd_print(2, 7, final_pos, 2);
						  if(Center_white_line>0x07)
						  {
							  flag=1;
							  forward();
							  velocity(160,160);
						  }

						  if((Left_white_line<0x07) && (flag==0))
						  {
							  flag=1;
							  forward();
							  velocity(130,50);
						  }

						  if((Right_white_line<=0x07) && (flag==0))
						  {
							  flag=1;
							  forward();
							  velocity(50,130);
						  }

						  if(Center_white_line<0x07 && Left_white_line<0x07 && Right_white_line<0x07)
						  {
							  forward();
							  velocity(160,160);
						  }
						  
						  if(Center_white_line>0x07 && Left_white_line>0x07 && Right_white_line>0x07)
						  {
							  final_pos = 4;;
							  lcd_print(2, 7, final_pos, 2);
							  node = 4;
							  lcd_print(2, 1, node, 2);
							  
							  buzzer_on();
							  _delay_ms(100);
							  buzzer_off();
							  _delay_ms(50);
							  forward();
							  velocity(150,150);
							  _delay_ms(700);
							  left();
							  _delay_ms(1900);
						  }
					  }
					  while ((final_pos == 10) && (node == 10))
					  {
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  
						  forward();
						  _delay_ms(100);
						  right();
						  _delay_ms(800);
						  stop();
						  final_pos = 103;
					  }
					  while (final_pos == 103)
					  {
						  Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
						  Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
						  Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
						  flag=0;

						  print_sensor(1,1,3);	//Prints value of White Line Sensor1
						  print_sensor(1,5,2);	//Prints Value of White Line Sensor2
						  print_sensor(1,9,1);	//Prints Value of White Line Sensor3
						  
						  lcd_print(2, 7, final_pos, 2);
						  if(Center_white_line>0x07)
						  {
							  flag=1;
							  forward();
							  velocity(160,160);
						  }

						  if((Left_white_line<0x07) && (flag==0))
						  {
							  flag=1;
							  forward();
							  velocity(130,50);
						  }

						  if((Right_white_line<=0x07) && (flag==0))
						  {
							  flag=1;
							  forward();
							  velocity(50,130);
						  }

						  if(Center_white_line<0x07 && Left_white_line<0x07 && Right_white_line<0x07)
						  {
							  forward();
							  velocity(160,160);
						  }
						  
						  if(Center_white_line>0x07 && Left_white_line>0x07 && Right_white_line>0x07)
						  {
							  final_pos = 19;
							  lcd_print(2, 7, final_pos, 2);
							  node = 19;
							  lcd_print(2, 1, node, 2);
							  
							  buzzer_on();
							  _delay_ms(100);
							  buzzer_off();
							  _delay_ms(50);
							  
							  forward();
							  velocity(150,150);
							  _delay_ms(600);
							  left();
							  _delay_ms(1300);
						  }
					  }
					  while ((final_pos == 24) && (node == 24))
					  {
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  buzzer_on();
						  _delay_ms(100);
						  buzzer_off();
						  _delay_ms(50);
						  
						  forward();
						  _delay_ms(400);
						  right();
						  _delay_ms(2100);
						  stop();
						  final_pos = 104;
					  }
					  while (final_pos == 104)
					  {
						  Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
						  Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
						  Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
						  flag=0;

						  print_sensor(1,1,3);	//Prints value of White Line Sensor1
						  print_sensor(1,5,2);	//Prints Value of White Line Sensor2
						  print_sensor(1,9,1);	//Prints Value of White Line Sensor3
						  
						  lcd_print(2, 7, final_pos, 2);
						  if(Center_white_line>0x07)
						  {
							  flag=1;
							  forward();
							  velocity(160,160);
						  }

						  if((Left_white_line<0x07) && (flag==0))
						  {
							  flag=1;
							  forward();
							  velocity(130,50);
						  }

						  if((Right_white_line<=0x07) && (flag==0))
						  {
							  flag=1;
							  forward();
							  velocity(50,130);
						  }

						  if(Center_white_line<0x0A && Left_white_line<0x0A && Right_white_line<0x0A)
						  {
							  forward();
							  velocity(160,160);
						  }
						  
						  if(Center_white_line>0x08 && Left_white_line>0x08 && Right_white_line>0x07)
						  {
							  final_pos = 9;
							  lcd_print(2, 7, final_pos, 2);
							  node = 9;
							  lcd_print(2, 1, node, 2);
							  
							  buzzer_on();
							  _delay_ms(100);
							  buzzer_off();
							  _delay_ms(50);

							  forward();
							  velocity(150,150);
							  _delay_ms(700);
							  right();
							  _delay_ms(1200);
						  }
					  }
					  
					
					while ((final_pos > node) && (final_pos != 0))
					{
						Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
						Center_white_line = ADC_Conversion(2);	//Getting data of Center WL Sensor
						Right_white_line = ADC_Conversion(1);	//Getting data of Right WL Sensor
						flag=0;

						print_sensor(1,1,3);	//Prints value of White Line Sensor1
						print_sensor(1,5,2);	//Prints Value of White Line Sensor2
						print_sensor(1,9,1);	//Prints Value of White Line Sensor3
						
						
						if(Center_white_line>0x07)
						{
							flag=1;
							forward();
							velocity(160,160);
						}

						if((Left_white_line<0x07) && (flag==0))
						{
							flag=1;
							forward();
							velocity(130,50);
						}

						if((Right_white_line<=0x07) && (flag==0))
						{
							flag=1;
							forward();
							velocity(50,130);
						}

						if(Center_white_line<0x07 && Left_white_line<0x07 && Right_white_line<0x07)
						{
							forward();
							velocity(160,160);
						}
						
						if(Center_white_line>0x07 && Left_white_line>0x07 && Right_white_line>0x07)
						{
							node = node+1;
							lcd_print(2, 1, node, 2);
							forward();
							_delay_ms(400);
							velocity(150,150);
							buzzer_on();
							_delay_ms(400);
							buzzer_off();
							
							
							if ((node == 5)||(node == 6)||(node == 15)||(node == 16))
							{
								forward();
								_delay_ms(100);
								left();
								_delay_ms(1300);
							}
							if ((node == 10)||(node == 11)||(node == 20)||(node == 21))
							{
								forward();
								_delay_ms(100);
								right();
								_delay_ms(1300);
							}
							if (node == 26)//changed this portion
							{
								forward();
								_delay_ms(100);
								stop();
								buzzer_on();
								_delay_ms(100);
								buzzer_off();
								_delay_ms(50);
								buzzer_on();
								_delay_ms(300);
								buzzer_off();
								_delay_ms(50);
								buzzer_on();
								_delay_ms(100);
								buzzer_off();
								_delay_ms(50);
								buzzer_on();
								_delay_ms(500);
								buzzer_off();
								_delay_ms(50);
								
								if(Center_white_line>0x07)
								{
									flag=1;
									forward();
									velocity(160,160);
								}

								if((Left_white_line<0x07) && (flag==0))
								{
									flag=1;
									forward();
									velocity(130,50);
								}

								if((Right_white_line<=0x07) && (flag==0))
								{
									flag=1;
									forward();
									velocity(50,130);
								}

								if(Center_white_line<0x07 && Left_white_line<0x07 && Right_white_line<0x07)
								{
									forward();
									velocity(160,160);
								}
								
								if(Center_white_line>0x07 && Left_white_line>0x07 && Right_white_line>0x07)
								{
									stop();
									_delay_ms(500);
									forward();
									_delay_ms(300);
									buzzer_on();
									left();
									_delay_ms(1500);
									stop();
									
									//Robo Dance
									
									int i=0;
									init_devices();
									motion_pin_config();
									bar_graph_led_pins_config();
									buzzer_pin_config();
									lcd_port_config();
									lcd_init();
									lcd_clear();
									lcd_home();
									
									forward();
									while(i<3)
									{
										buzzer_on();
										turn_on_bar_graph_led(bar_graph_led_1_pin);
										turn_on_bar_graph_led(bar_graph_led_3_pin);
										turn_on_bar_graph_led(bar_graph_led_5_pin);
										turn_on_bar_graph_led(bar_graph_led_7_pin);
										_delay_ms(150);
										turn_off_bar_graph_led(bar_graph_led_1_pin);
										turn_off_bar_graph_led(bar_graph_led_3_pin);
										turn_off_bar_graph_led(bar_graph_led_5_pin);
										turn_off_bar_graph_led(bar_graph_led_7_pin);
										buzzer_off();
										turn_on_bar_graph_led(bar_graph_led_2_pin);
										turn_on_bar_graph_led(bar_graph_led_4_pin);
										turn_on_bar_graph_led(bar_graph_led_6_pin);
										turn_on_bar_graph_led(bar_graph_led_8_pin);
										_delay_ms(150);
										turn_off_bar_graph_led(bar_graph_led_2_pin);
										turn_off_bar_graph_led(bar_graph_led_4_pin);
										turn_off_bar_graph_led(bar_graph_led_6_pin);
										turn_off_bar_graph_led(bar_graph_led_8_pin);
										i++;
									}
									_delay_ms(100);
									stop();
									_delay_ms(500);
									buzzer_on();
									right();
									_delay_ms(740);
									stop();
									buzzer_off();
									_delay_ms(500);
									buzzer_on();
									left();
									_delay_ms(1480);
									stop();
									buzzer_off();
									_delay_ms(500);
									buzzer_on();
									right();
									_delay_ms(740);
									stop();
									buzzer_off();
									_delay_ms(500);
									
									for(int i=0;i<3;i++)
									{
										static_loop2();
										_delay_ms(350);
										lcd_clear();
										_delay_ms(400);
									}
									
									while(1)
									{
										static_loop();
									}
								}
							}
						}
					}
					stop();
		}
		
	}
}





