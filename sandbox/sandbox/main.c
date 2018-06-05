/*
 * sandbox.c
 *
 * Created: 5/24/2018 10:27:23 AM
 * Author : David
 */ 


#include <stdlib.h>
#include <string.h>
#include <math.h>  //include libm
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>

//#define usart_BAUD_RATE 9600
#include <ucr/usart.h>

#include <wiinunchuck/wiinunchuck.h>
#include <wiinunchuck/wiinunchuck.c>

enum BTN_States{BTN_start, BTN_wait, BTN_Z, BTN_C} BTN_state = 0;
	
 void tickFct() {
 	switch (BTN_state) {
 		case BTN_start:
 			BTN_state = BTN_wait;
 			break;
 		case BTN_wait:
 			if (wiinunchuck_getbuttonC()) {
 				BTN_state = BTN_C;
 			} else if (wiinunchuck_getbuttonZ()) {
 				BTN_state = BTN_Z;
 			}
 			break;
 		case BTN_Z:
 			if (wiinunchuck_getbuttonZ()) {
 				BTN_state = BTN_Z;
 			} else {
 				BTN_state = BTN_wait;
 			}
 			break;
 		case BTN_C:
 			if (wiinunchuck_getbuttonC()) {
 				BTN_state = BTN_C;
 			} else {
 				BTN_state = BTN_wait;
 			}
 			break;
 		default:
 			BTN_state = BTN_wait;
 	}
 	switch (BTN_state) {
 		case BTN_wait:
 			PORTA = 0x00;
 			break;
 		case BTN_Z:
 			PORTA = ~0x01;
 			break;
 		case BTN_C:
 			PORTA = ~0x02;
 			break;
 	}
 }

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	//DDRC = 0x00; PORTC = 0xFF;
	//initUSART();
	wiinunchuck_init();
	//sei();
	//data vars
	uint8_t buttonZ;
	uint8_t buttonC;
	int joyX;
	int joyY;
	int angleX;
	int angleY;
	int angleZ;
    /* Replace with your application code */
    while (1) 
    {
		
		//update data
		wiinunchuck_update();
//
		////get data
		//buttonZ = wiinunchuck_getbuttonZ();
		//buttonC = wiinunchuck_getbuttonC();
		//joyX = wiinunchuck_getjoyX();
		//joyY = wiinunchuck_getjoyY();
		//angleX = wiinunchuck_getangleX();
		//angleY = wiinunchuck_getangleY();
		//angleZ = wiinunchuck_getangleZ();
		////tickFct();
		

    }
}

