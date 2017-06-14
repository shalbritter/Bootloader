/*
 * Bootloader.c
 *
 * Created: 08.06.2017 12:45:00
 * Author : lid32272
 */ 

/* TODO
 * linker -Ttext=ByteAdress
 * set fuses
 * 
 */

//include//////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

//define///////////////////////////////////////////////////////////////////////////
#define XOFF 0x13
#define XON 0x11
#define UART_BUFFER_LENGTH 16
#define UART_BUFFER_MIN_EMPTY 4

//UART////////////////////////////////////////////////////////////////////////////
volatile char UART_in[UART_BUFFER_LENGTH] = {'\0'};
volatile uint8_t UART_in_read = 0;
volatile uint8_t UART_in_receive = 0;

//UART Flags
volatile uint8_t UART_Flags = 0x00;
#define UART_RECEIVE_BLOCKED 0
#define UART_SEND_BLOCKED 1
#define UART_WRITING 2

ISR(USART_RX_vect){		//TODO check what is necessary for getc
	UART_in[UART_in_receive] = UDR0;

	if (UART_in[UART_in_receive] == XOFF)
	{
		UART_Flags |= (1<<UART_SEND_BLOCKED);
	} else if (UART_in[UART_in_receive] == XON)
	{
		UART_Flags &= ~(1<<UART_SEND_BLOCKED);
	}
	UART_in_receive = (UART_in_receive + 1) % UART_BUFFER_LENGTH;
	//if(((UART_in_receive + UART_BUFFER_LENGTH - UART_in_read) % UART_BUFFER_LENGTH ) < UART_BUFFER_MIN_EMPTY){ //Buffer almost full
		putc(XOFF);	//TODO fix receive control
		//UART_Flags |= (1<<UART_RECEIVE_BLOCKED);
	//}
}

ISR(USART_TX_vect){
	UART_Flags &= ~(1<<UART_WRITING);
}

//prototypes///////////////////////////////////////////////////////////////////////
uint8_t get_byte();
uint16_t get_word();
uint16_t get_word_inv();
void activate_interrupts();
void deactivate_interrupts();
void initUART(uint8_t ubbr);
void deinitUART();
char getc();
void putc(char data);
uint16_t atoi_hex(const char *__s);
char* reverse(char *__s, int __len);
char* itoa(int __val, char *__s, int __radix);

//main/////////////////////////////////////////////////////////////////////////////
int main(void)
{
	DDRD = 0x04;	//init button on PD2
	PORTD = 0x04;
	DDRB = 0x01;
	PORTB = 0x01;
	
	if(!(PIND & (1<<PIND2))){	//start bootloader if button is pressed
	
		activate_interrupts();
		initUART(103); //init UART to 9600 baud
		putc(XON);
	
		//red light on during bootloading
		PORTB = 0x00;
		
		//256 pages of 128 bytes
		uint16_t page = 0;
		while (page <= 256) {
			uint8_t word[SPM_PAGESIZE] = {0};
			char __c = getc();
			if(__c == ':'){	//new line
				uint8_t length = get_word();
			}
			//TODO do bootloader stuff here
			page++;
		}
		
		
	
		deactivate_interrupts();
		deinitUART();
		DDRD = 0x00;	//deinit PD2
		PORTD = 0x00;
	}
	//end boootloader
	void (*start) ( void ) = 0x0000;
	start();
}

//functions////////////////////////////////////////////////////////////////////////
uint8_t get_byte(){
	char byte_c[2];
	byte_c[0] = getc();
	byte_c[1] = getc();
	return atoi_hex(byte_c);
}

uint16_t get_word(){
	return 0x10 * get_byte() + get_byte();
}

uint16_t get_word_inv(){
	uint8_t temp = get_byte();
	return 0x10 * get_byte() + temp;
}

void activate_interrupts(){
	uint8_t temp = MCUCR;
	MCUCR = temp | (1<<IVCE);
	MCUCR = temp | (1<<IVSEL);
	sei();
}

void deactivate_interrupts(){
	uint8_t temp = MCUCR;
	MCUCR = temp | (1<<IVCE);
	MCUCR = temp & ~(1<<IVSEL);
	cli();
}

void initUART(uint8_t ubbr){
	UBRR0 = ubbr;
	UCSR0B |= ((1<<RXEN0) | (1<<TXEN0));	//Enable TxD and RxD
	UCSR0B |= ((1<<RXCIE0) | (1<<TXCIE0));
}

void deinitUART(){	//reset to initial valuess
	UBRR0 = 0x00;
	UCSR0B = 0x00; 
} 

char getc(){
	char returnc = 0x00;
	if(UART_in_read != UART_in_receive) {
		returnc = UART_in[UART_in_read];
		UART_in_read = (UART_in_read + 1) % UART_BUFFER_LENGTH;
	}
	return returnc;
}

void putc(char data){
	while(UART_Flags & (1<<UART_SEND_BLOCKED)) ;
	while(UART_Flags & (1<<UART_WRITING))
	;
	UART_Flags |= (1<<UART_WRITING);
	UDR0 = data;
}

uint16_t atoi_hex(const char *__s) {
	uint16_t ret = 0;
	while (*(__s+1))
	{
		if ('0' <= *__s && *__s <= '9')
		{
			ret += (*__s - '0');
		}
		else if ('A' <= *__s && *__s <= 'F')
		{
			ret += (*__s - 'A' + 10);
		}
		ret *= 16;
		__s++;
	}
	if ('0' <= *__s && *__s <= '9')
	{
		ret += (*__s - '0');
	}
	else if ('A' <= *__s && *__s <= 'F')
	{
		ret += (*__s - 'A' + 10);
	}
	return ret;
}

char* reverse(char *__s, int __len) {
	int start = 0, end = __len - 1;
	while (start < end)
	{
		char temp = __s[start];
		__s[start] = __s[end];
		__s[end] = temp;
		start++;
		end--;
	}
	return __s;
}

char* itoa(int __val, char *__s, int __radix) {
	int i = 0;
	if (__val == 0)
	{
		__s[i++] = '0';
		__s[i] = '\0;';
		return __s;
	}
	while (__val != 0)
	{
		int rem = __val % __radix;
		__s[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		__val = __val / __radix;
	}

	__s[i] = '\0';
	__s = reverse(__s, i);
	return __s;
}
