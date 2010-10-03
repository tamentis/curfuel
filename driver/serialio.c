#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// #include "wiring.h"
#include "wiring_private.h"
#include "serialio.h"

struct ring_buffer rx_buffer = { { 0 }, 0, 0 };

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
struct ring_buffer rx_buffer1 = { { 0 }, 0, 0 };
struct ring_buffer rx_buffer2 = { { 0 }, 0, 0 };
struct ring_buffer rx_buffer3 = { { 0 }, 0, 0 };
#endif

extern struct ring_buffer rx_buffer;

void
store_char(unsigned char c, struct ring_buffer *rx_buffer)
{
	int i = (rx_buffer->head + 1) % RX_BUFFER_SIZE;

	/* 
	 * If we should be storing the received character into the location
	 * just before the tail (meaning that the head would advance to the
	 * current location of the tail), we're about to overflow the buffer
	 * and so we don't write the character or advance the head.
	 */
	if (i != rx_buffer->tail) {
		rx_buffer->buffer[rx_buffer->head] = c;
		rx_buffer->head = i;
	}
}

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

SIGNAL(SIG_USART0_RECV)
{
	unsigned char c = UDR0;
	store_char(c, &rx_buffer);
}

SIGNAL(SIG_USART1_RECV)
{
	unsigned char c = UDR1;
	store_char(c, &rx_buffer1);
}

SIGNAL(SIG_USART2_RECV)
{
	unsigned char c = UDR2;
	store_char(c, &rx_buffer2);
}

SIGNAL(SIG_USART3_RECV)
{
	unsigned char c = UDR3;
	store_char(c, &rx_buffer3);
}

#else

#if defined(__AVR_ATmega8__)
SIGNAL(SIG_UART_RECV)
#else
SIGNAL(USART_RX_vect)
#endif
{
#if defined(__AVR_ATmega8__)
	unsigned char c = UDR;
#else
	unsigned char c = UDR0;
#endif
	store_char(c, &rx_buffer);
}

#endif

void
serial_setup(serial *port, volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
		volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
		volatile uint8_t *udr, uint8_t rxen, uint8_t txen,
		uint8_t rxcie, uint8_t udre, uint8_t u2x)
{
	port->rx_buffer = &rx_buffer;
	port->ubrrh = ubrrh;
	port->ubrrl = ubrrl;
	port->ucsra = ucsra;
	port->ucsrb = ucsrb;
	port->udr = udr;
	port->rxen = rxen;
	port->txen = txen;
	port->rxcie = rxcie;
	port->udre = udre;
	port->u2x = u2x;
}

void
serial_begin(serial *port, long baud)
{
	uint16_t baud_setting;
	int use_u2x;
	
	// U2X mode is needed for baud rates higher than (CPU Hz / 16)
	if (baud > F_CPU / 16) {
		use_u2x = 0x1;
	} else {
	  // figure out if U2X mode would allow for a better connection
	  
	  // calculate the percent difference between the baud-rate specified
	  // and the real baud rate for both U2X and non-U2X mode (0-255 error
	  // percent)
	  uint8_t nonu2x_baud_error = abs((int)(255-((F_CPU/(16*(((F_CPU/8/baud-1)/2)+1))*255)/baud)));
	  uint8_t u2x_baud_error = abs((int)(255-((F_CPU/(8*(((F_CPU/4/baud-1)/2)+1))*255)/baud)));
	  
	  // prefer non-U2X mode because it handles clock skew better
	  use_u2x = (nonu2x_baud_error > u2x_baud_error);
	}
	
	if (use_u2x) {
	  *(port->ucsra) = 1 << port->u2x;
	  baud_setting = (F_CPU / 4 / baud - 1) / 2;
	} else {
	  *(port->ucsra) = 0;
	  baud_setting = (F_CPU / 8 / baud - 1) / 2;
	}
	
	/* Assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register) */
	*(port->ubrrh) = baud_setting >> 8;
	*(port->ubrrl) = baud_setting;
	
	sbi(*(port->ucsrb), port->rxen);
	sbi(*(port->ucsrb), port->txen);
	sbi(*(port->ucsrb), port->rxcie);
}

void
serial_end(serial *port)
{
	cbi(*(port->ucsrb), port->rxen);
	cbi(*(port->ucsrb), port->txen);
	cbi(*(port->ucsrb), port->rxcie);  
}

int
serial_pending(serial *port)
{
	return (RX_BUFFER_SIZE + port->rx_buffer->head - port->rx_buffer->tail) %
			RX_BUFFER_SIZE;
}

int
serial_peek(serial *port)
{
	if (port->rx_buffer->head == port->rx_buffer->tail) {
		return -1;
	} else {
		return port->rx_buffer->buffer[port->rx_buffer->tail];
	}
}

int
serial_read(serial *port)
{
	/* if the head isn't ahead of the tail, we don't have any characters */
	if (port->rx_buffer->head == port->rx_buffer->tail) {
		return -1;
	} else {
		unsigned char c = port->rx_buffer->buffer[port->rx_buffer->tail];
		port->rx_buffer->tail = (port->rx_buffer->tail + 1) % RX_BUFFER_SIZE;
		return c;
	}
}

void
serial_flush(serial *port)
{
	/*
	 * don't reverse this or there may be problems if the RX
	 * interrupt occurs after reading the value of rx_buffer_head
	 * but before writing the value to rx_buffer_tail; the previous
	 * value of rx_buffer_head may be written to rx_buffer_tail,
	 * making it appear as if the buffer don't reverse this or there
	 * may be problems if the RX interrupt occurs after reading the
	 * value of rx_buffer_head but before writing the value to
	 * rx_buffer_tail; the previous value of rx_buffer_head may be
	 * written to rx_buffer_tail, making it appear as if the buffer
	 * were full, not empty.
	 */
	port->rx_buffer->head = port->rx_buffer->tail;
}

void
serial_write(serial *port, uint8_t c)
{
	while (!((*(port->ucsra)) & (1 << (port->udre))))
		;

	*(port->udr) = c;
}

void
serial_print(serial *port, const char *str)
{
	while (*str)
		serial_write(port, *str++);
}


void
serial_nprint(serial *port, const uint8_t *buffer, size_t size)
{
	while (size--)
		serial_write(port, *buffer++);
}

void
serial_print_double(serial *port, double number, uint8_t digits) 
{ 
	uint8_t i;

	/* Handle negative numbers */
	if (number < 0.0) {
		serial_write(port, '-');
		number = -number;
	}

	/* Round correctly so that print(1.999, 2) prints as "2.00" */
	double rounding = 0.5;
	for (i = 0; i < digits; ++i)
		rounding /= 10.0;
  
	number += rounding;

	/* Extract the integer part of the number and print it */
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	serial_print_ulong(port, int_part, 10);

	/* Print the decimal point, but only if there are digits beyond */
	if (digits > 0)
		serial_write(port, '.');

	/* Extract digits from the remainder one at a time */
	while (digits-- > 0) {
		remainder *= 10.0;
		int to_print = (int)remainder;
		serial_print_ulong(port, (unsigned long)to_print, 10);
		remainder -= to_print; 
	} 
}

void
serial_print_long(serial *port, long n, uint8_t base)
{
	if (base == 0) {
		serial_write(port, n);
	} else if (base == 10) {
		if (n < 0) {
			print(port, "-");
			n = -n;
		}
		print_number(port, n, 10);
	} else {
		print_number(port, n, base);
	}
}

void
serial_print_ulong(serial *port, unsigned long n, uint8_t base)
{
	unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
	unsigned long i = 0;
	
	if (n == 0) {
		serial_write(port, '0');
		return;
	} 
	
	while (n > 0) {
		buf[i++] = n % base;
		n /= base;
	}
	
	for (; i > 0; i--) {
		serial_write(port, (char) (buf[i - 1] < 10 ?
			'0' + buf[i - 1] :
			'A' + buf[i - 1] - 10));
	}
}

