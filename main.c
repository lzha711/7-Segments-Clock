/*
 * 7SegmentDisplay.c
 * 
 * Created: 5/28/2021 10:10:48 AM
 * Author : lzha711
 */ 

// 16MHz external xtal
#define F_CPU 16000000UL
#define true 1
#define false 0
#define POV_delay 15 //15ms works fine

// library
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

//declare variables and functions
void CalculateMinutes (void);
void CalculateSeconds (void);
void WriteDisplays (uint8_t digit, uint8_t displaynum);
void IO_init(void);
void TIMER1_init(void);

uint8_t minutes = 0;
uint8_t seconds = 0;

//a-g 7 segment values for each number
uint8_t seven_seg_digits[10][7] = {
	{ 1,1,1,1,1,1,0 },  // = 0
	{ 0,1,1,0,0,0,0 },  // = 1
	{ 1,1,0,1,1,0,1 },  // = 2
	{ 1,1,1,1,0,0,1 },  // = 3
	{ 0,1,1,0,0,1,1 },  // = 4
	{ 1,0,1,1,0,1,1 },  // = 5
	{ 1,0,1,1,1,1,1 },  // = 6
	{ 1,1,1,0,0,0,0 },  // = 7
	{ 1,1,1,1,1,1,1 },  // = 8
	{ 1,1,1,1,0,1,1 }   // = 9
};

void WriteDisplays(uint8_t digit, uint8_t displaynum){
	uint8_t PD_pin = 1; // initialize to PD1 (PD1 = 1)
	PORTD &= 0b00000000; //clear port D
	PORTC &= ~(1<<PC0); // clear PC0
	
	// display select
	if (displaynum == 0){
		PORTB &= ~(1<<PB2); //select Sec2 display
		PORTB |= (1<<PB3) | (1<<PB4) | (1<<PB5); //disable all other modules
	}else if (displaynum == 1){
		PORTB &= ~(1<<PB3); //select Sec1 display
		PORTB |= (1<<PB2) | (1<<PB4) | (1<<PB5); //disable all other modules
	}else if (displaynum == 2){
		PORTB &= ~(1<<PB4); //select Min2
		PORTB |= (1<<PB2) | (1<<PB3) | (1<<PB5); //disable all other modules
	}else if (displaynum == 3){
		PORTB &= ~(1<<PB5); //select Min1
		PORTB |= (1<<PB2) | (1<<PB3) | (1<<PB4); //disable all other modules
	}
	
	// assign display number
	for (int segCount = 0; segCount <7; ++segCount){
		if(PD_pin < 8){
			PORTD |= (seven_seg_digits[digit][segCount]<<PD_pin); //assign [digit][a-f] to PD1-PD7
			//PORTC |= (seven_seg_digits[digit][6]<<PC0);//assign [digit][g] to PC0
		}
		else{
			// PORTC |= (seven_seg_digits[digit][segCount]<<PC0);//assign [digit][g] to PC0
		}
		//the simulation shows that g displays slower than everyone else 
		++PD_pin;
		// interesting phenomenon: when using PD1-PD7, the led shows good. when assigning g to another IO, PB0 or PC0, the timing of g becomes wrong
	}
}

void CalculateSeconds(void){
	if (seconds/10 == 0 && seconds < 10){
			WriteDisplays(0,1);
			_delay_ms(POV_delay);
			WriteDisplays(seconds,0);
			_delay_ms(POV_delay);
	}else if (seconds%10 != 0){
			WriteDisplays(seconds/10,1);
			_delay_ms(POV_delay);
			WriteDisplays(seconds%10,0);
			_delay_ms(POV_delay);
	}else if (seconds%10 == 0){
			WriteDisplays(seconds/10,1);
			_delay_ms(POV_delay);
			WriteDisplays(0,0);
			_delay_ms(POV_delay);
	} 
}

void CalculateMinutes(void){
		if (minutes/10 == 0 && minutes < 10){
			WriteDisplays(0,3);
			_delay_ms(POV_delay);
			WriteDisplays(minutes,2);
			_delay_ms(POV_delay);
		}else if (minutes%10 != 0){
			WriteDisplays(minutes/10,3);
			_delay_ms(POV_delay);
			WriteDisplays(minutes%10,2);
			_delay_ms(POV_delay);
		}else if (minutes%10 == 0){
			WriteDisplays(minutes/10,3);
			_delay_ms(POV_delay);
			WriteDisplays(0,2);
			_delay_ms(POV_delay);
	    }
}


void IO_init(void){
	DDRD = 0b11111100; //set PD2-PD7 as output
	DDRB = 0b00111111; //set PB0-PB5 as output
	DDRC |= (1<<PC0); // set PC0 as output
}

void TIMER1_init(void){
    TCCR1A = 0b00000000; //normal mode
	TCCR1B = 0b00001101; //CTC mode, clk_t0 = clk_io/1024
	OCR1A = 0x3D08; // trigger interrupt every 1s
	TIMSK1 = 0b00000010; // set bit OCIE1A, timer/counter1 output compare match A interrupt enable
	PORTB |= (1<<PB1); //set OC1A as output, doesn't matter input or output
}


// execute every 1s
ISR(TIMER1_COMPA_vect){
	//store value here only
	if (seconds > 59)
	{
		minutes+=1;
		seconds = 0;
	}
	if (minutes > 59)
	{
		minutes = 0;
		seconds = 0;
	}
	seconds++;
}


int main(void)
{
   // uint8_t pin, digit=0;
	IO_init();
	TIMER1_init();
	sei(); //enable interrupt

    while (1) 
    {	
		CalculateSeconds();
		CalculateMinutes();
    }
}

