/*
 * Define constants and variables for buffering incoming serial data.
 * We're using a ring buffer (I think), in which rx_buffer_head is the
 * index of the location to which to write the next incoming character
 * and rx_buffer_tail is the index of the location from which to read.
 */
#define RX_BUFFER_SIZE 128

struct ring_buffer {
	unsigned char	 buffer[RX_BUFFER_SIZE];
	int		 head;
	int		 tail;
};


typedef struct serial_port {
	struct			 ring_buffer *rx_buffer;
	volatile uint8_t	*ubrrh;
	volatile uint8_t	*ubrrl;
	volatile uint8_t	*ucsra;
	volatile uint8_t	*ucsrb;
	volatile uint8_t	*udr;
	uint8_t			 rxen;
	uint8_t			 txen;
	uint8_t			 rxcie;
	uint8_t			 udre;
	uint8_t			 u2x;
} serial;

void	serial_setup(serial *, volatile uint8_t *, volatile uint8_t *,
		volatile uint8_t *, volatile uint8_t *, volatile uint8_t *,
		uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void	serial_begin(serial *, long);
void	serial_end(serial *);
int	serial_pending(serial *);
int	serial_peek(serial *);
int	serial_read(serial *);
void	serial_flush(serial *);
void	serial_write(serial *, uint8_t);
void	serial_print(serial *, const char *);
void	serial_nprint(serial *, const uint8_t *, size_t);
void	serial_print_double(serial *, double, uint8_t);
void	serial_print_ulong(serial *, unsigned long, uint8_t);
void	serial_print_long(serial *, long, uint8_t);

