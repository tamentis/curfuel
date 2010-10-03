/*
 * Dump the eeprom to the serial port.
 */

#include "wiring_private.h"
#include "serialio.h"
// #include "WProgram.h"


int
main(void)
{
	int address = 0;

	serial port;
	setup_serial(&port, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UDR0, RXEN0,
			TXEN0, RXCIE0, UDRE0, U2X0);

	init();

	begin(&port, 9600);

	pinMode(13, OUTPUT);
    
	for (;;) {
		digitalWrite(13, HIGH);
		delay(500);
		digitalWrite(13, LOW);
		delay(500);
		digitalWrite(13, HIGH);
		delay(500);
		digitalWrite(13, LOW);
		delay(500);
		// print(&port, "hello world!\n");
		write(&port, '-');
		write(&port, 'h');
		delay(1);
		write(&port, 'e');
		write(&port, 'l');
		delay(1);
		write(&port, 'l');
		write(&port, 'o');
		delay(1);
		write(&port, '-');
		write(&port, '\n');
		delay(500);
		digitalWrite(13, LOW);
		delay(500);
	}

	end(&port);
        
	return 0;
}


