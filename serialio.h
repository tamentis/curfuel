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

void	store_char(unsigned char, struct ring_buffer *);
void	setup_serial(serial *, volatile uint8_t *, volatile uint8_t *,
		volatile uint8_t *, volatile uint8_t *, volatile uint8_t *,
		uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void	begin(serial *, long);
void	end(serial *);
int	available(serial *);
int	peek(serial *);
int	read(serial *);
void	flush(serial *);
void	write(serial *, uint8_t);
void	print(serial *, const char *);
void	nprint(serial *, const uint8_t *, size_t);
void	print_float(serial *, double, uint8_t);
void	print_byte(serial *, char, int);
void	print_ubyte(serial *, unsigned char, int);
void	print_int(serial *, int, int);
void	print_uint(serial *, unsigned int, int);
void	print_long(serial *, long, int);
void	print_ulong(serial *, unsigned long, int);
void	print_number(serial *, unsigned long, uint8_t);

