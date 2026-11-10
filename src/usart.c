#include <avr/io.h>
#include <stdio.h>
#include <stdarg.h>
#include "usart.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 19200
#endif
#include <util/setbaud.h>
#include <util/delay.h>

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

void uart_init(void) {

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

#if USE_NEXTION

static void send_byte(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

static void send_end(void) {            /* Nextion end-of-command marker */
	send_byte(0xFF); send_byte(0xFF); send_byte(0xFF);
}

static void send_text(const char *s) {
	while (*s) send_byte(*s++);
}

/* printf text is collected until '\n', then sent as  tN.txt="line"
   Lines go to t0, t1, ... in turn and wrap around after NEX_LINES. */
#define LINE_MAX 40
static char line_buf[LINE_MAX];
static uint8_t line_len = 0;
static uint8_t line_no = 0;

int uart_putchar(char c, FILE *stream) {
	(void)stream;

	if (c == '\r') return 0;

	if (c == '\n') {
		line_buf[line_len] = '\0';
		send_byte('t');
		send_byte('0' + line_no);
		send_text(".txt=\"");
		send_text(line_buf);
		send_byte('"');
		send_end();

		line_len = 0;
		line_no = (line_no + 1) % NEX_LINES;
		return 0;
	}

	if (c == '"') c = '\'';              /* a quote would break the command */
	if (line_len < LINE_MAX - 1) line_buf[line_len++] = c;
	return 0;
}

void nex_text(const char *obj, const char *fmt, ...) {
	char buf[LINE_MAX];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	for (char *p = buf; *p; p++) {
		if (*p == '"') *p = '\'';           /* a quote would break the command */
	}

	while (*obj) send_byte(*obj++);
	send_text(".txt=\"");
	send_text(buf);
	send_byte('"');
	send_end();
}

int uart_getchar(FILE *stream) {         /* not usable with the display (RX is used by the slider) */
	(void)stream;
	return -1;
}

void io_redirect(void) {
	stdout = &uart_output; //redirect standard output to uart
	stdin  = &uart_input; //redirect standard input to uart

	_delay_ms(500);
	send_end();
	_delay_ms(100);
}

#else   /* ---- plain PC serial output ---- */

int uart_putchar(char c, FILE *stream) {
	if (c == '\n') {
		uart_putchar('\r', stream);
	}
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

int uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

void nex_text(const char *obj, const char *fmt, ...) {
	char buf[40];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printf("%s: %s\n", obj, buf);
}

void io_redirect(void) {
	stdout = &uart_output;	//redirect standard output to uart
	stdin  = &uart_input;  //redirect standard input to uart
}

#endif
