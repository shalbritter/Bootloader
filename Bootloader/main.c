/*
 * Bootloader.c
 *
 * Created: 08.06.2017 12:45:00
 * Author : x10mi
 */ 

//include//////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

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
	if(((UART_in_receive + UART_BUFFER_LENGTH - UART_in_read) % UART_BUFFER_LENGTH ) < UART_BUFFER_MIN_EMPTY){ //Buffer almost full
		//UART_Transmit_c(XOFF);	//TODO fix receive control
		UART_Flags |= (1<<UART_RECEIVE_BLOCKED);
	}
}

//prototypes///////////////////////////////////////////////////////////////////////
uint16_t atoi_hex(const char *__s);
char* reverse(char *__s, int __len);
char* itoa(int __val, char *__s, int __radix);



//main/////////////////////////////////////////////////////////////////////////////
int main(void)
{
	DDRD = 0x04;	//init button on PD2
	PORTD = 0x04;
	
    initUART(103); //init UART to 9600 baud
	UART_Transmit_c(XON);
    
	while (1) 
    {
    }
}

//functions////////////////////////////////////////////////////////////////////////
void initUART(uint8_t ubbr){
	UBRR0 = ubbr;
	UCSR0B |= ((1<<RXEN0) | (1<<TXEN0));	//Enable TxD and RxD
	UCSR0B |= ((1<<RXCIE0) | (1<<TXCIE0));	
}   

char UART_getc(){
	char returnc = 0x00;
	if(UART_in_read != UART_in_receive) {
		returnc = UART_in[UART_in_read];
		UART_in_read = (UART_in_read + 1) % UART_BUFFER_LENGTH;
	}
	return returnc;
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
