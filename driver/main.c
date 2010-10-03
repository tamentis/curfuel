/*
 * Dump the eeprom to the serial port.
 */

#include "wiring_private.h"
#include "serialio.h"

/* Number of samples to take */
#define SAMPLING_COUNT 10
#define SAMPLING_DELAY 10

/* Setup the serial port for my ATmega328p */
void
setup_atmega328p_serial(serial *port)
{
	serial_setup(port, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UDR0, RXEN0,
			TXEN0, RXCIE0, UDRE0, U2X0);
}

/* Average a few samples to get a more precise reading. */
double
curfuel_get_avg()
{
	int i;
	unsigned long sum = 0;
	int sensor;

	for (i = 0; i < SAMPLING_COUNT; i++) {
		sum += analogRead(A0);
		delay(SAMPLING_DELAY);
	}

	return (double)sum / (double)SAMPLING_COUNT;
}

/* Print the header for commands */
void
curfuel_print_header(serial *port, char *name)
{
	serial_print_ulong(port, millis(), 10);
	serial_write(port, ':');
	serial_print(port, name);
	serial_write(port, ':');
}

/* Print the raw fuel level */
void
curfuel_print(serial *port, double avg)
{
	curfuel_print_header(port, "raw_fuel_level");
	serial_print_double(port, avg, 6);
	serial_write(port, '\n');
}

/* Print an unknown command error message */
void
curfuel_print_unknown(serial *port, unsigned char c)
{
	curfuel_print_header(port, "unknown_command");
	serial_write(port, c);
	serial_write(port, '\n');
}

int
main(void)
{
	uint8_t query;
	double avg;
	serial port;
	
	setup_atmega328p_serial(&port);

	init();

	pinMode(13, OUTPUT);
	serial_begin(&port, 9600);

	for (;;) {
		delay(100);

		if (serial_pending(&port) == 0)
			continue;

		query = serial_read(&port);
		switch (query) {
			case 'f':
				avg = curfuel_get_avg();
				curfuel_print(&port, avg);
				break;
			default:
				curfuel_print_unknown(&port, query);
				break;
		}

		/* 
		 * Do not let a queue accumulate, if a client begs for
		 * too much, too fast, screw them.
		 */
		serial_flush(&port);
	}
	serial_end(&port);
        
	return 0;
}


