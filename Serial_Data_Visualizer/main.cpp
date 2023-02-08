#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include <stdio.h>
#include <stdlib.h>

#define LTHRES 500
#define RTHRES 500


//Define Baud
#define USART_BAUDRATE 19200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

volatile uint8_t value = 0;// volatile so both main and RX interrupt can use it
volatile uint8_t newData = 0;

ISR(USART_RX_vect)
{
	value = UDR0;
	
	newData = 1; //For Loopback
}


void USART_Init(void)
{
	// Set baud rate
	UBRR0L = BAUD_PRESCALE;//lower 8-bits into the the UBRRL register
	UBRR0H = (BAUD_PRESCALE >> 8); //upper 8-bits into the UBRRH register
	UCSR0B = ((1<<TXEN0)|(1<<RXEN0) | (1<<RXCIE0));//enable tx and Rx. Receive interrupt
	
	/* Default frame format is 8 data bits, no parity, 1 stop bit to change use UCSRC, see AVR datasheet*/
}





// initialize adc
void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
	
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}



void USART_SendByte(char data){

	// Wait until last byte has been transmitted
	while((UCSR0A &(1<<UDRE0)) == 0); //0010 0000

	// Transmit data
	UDR0 = data;
}




uint8_t USART_ReceiveByte(){
	while((UCSR0A &(1<<RXC0)) == 0);
	return UDR0;
}



int main()
{
	uint16_t adc_result0;
	char int_buffer[10];
	DDRC = 0x01;           // to connect led to PC0
	
	// initialize adc and lcd


	USART_Init();
	adc_init();
	sei();
	USART_SendByte(0x49);
	USART_SendByte('O');
	USART_SendByte(0b01010100);
	USART_SendByte(45);
	USART_SendByte(0x41);
	USART_SendByte('R');
	USART_SendByte(0x4D);
	USART_SendByte(0b01011001);

	
	_delay_ms(50);
	


	while(1)
	{
		adc_result0 = adc_read(0);      // read adc value at PA0
		// read adc value at PA1
		
		itoa(adc_result0, int_buffer, 10);

		for (int c=0; c<=10; c++)
		{
			USART_SendByte(int_buffer[c]);
			
		}
		
		_delay_ms(500);
		
	}
}