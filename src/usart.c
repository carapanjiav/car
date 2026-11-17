#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
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
    UCSR0A &= ~_BV(U2X0);
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8 data bits, 1 stop bit, no parity
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   // Enable receiver and transmitter

#if USE_NEXTION
    UCSR0B |= _BV(RXCIE0);             // Receive bytes asynchronously
    sei();
#endif
}

#define LINE_MAX 80

#if USE_NEXTION

#define RX_SIZE 64

static void send_byte(uint8_t byte){
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = byte;
}

static void send_end(void) {            /* Nextion end-of-command marker */
	send_byte(0xFF); send_byte(0xFF); send_byte(0xFF);
}

static void send_text(const char *s) {
	while (*s) send_byte((uint8_t)*s++);
}

/* printf text is collected until '\n', then sent as  tN.txt="line"
   Lines go to t0, t1, ... in turn and wrap around after NEX_LINES. */
static volatile uint8_t rx_buf[RX_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

ISR(USART_RX_vect) {
    uint8_t byte = UDR0;
    uint8_t next = (uint8_t)((rx_head + 1U) % RX_SIZE);

    if (next != rx_tail) {
        rx_buf[rx_head] = byte;
        rx_head = next;
    }
}

static char line_buf[LINE_MAX];
static uint8_t line_len = 0;
static uint8_t line_no = 0;

int uart_putchar(char c, FILE *stream) {
	(void)stream;

	if (c == '\r') return 0;

	if (c == '\n') {
		line_buf[line_len] = '\0';
		send_byte('t');
		send_byte((uint8_t)('0' + line_no));
		send_text(".txt=\"");
		send_text(line_buf);
		send_byte('"');
		send_end();

		line_len = 0;
        line_no = (uint8_t)((line_no + 1U) % NEX_LINES);
		return 0;
	}

	if (c == '"') c = '\'';              /* a quote would break the command */
    if ((uint8_t)line_len < LINE_MAX - 1U) line_buf[line_len++] = c;
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

void nex_cmd(const char *fmt, ...) {
    char buf[LINE_MAX];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    send_text(buf);
    send_end();
}

void nex_val(const char *obj, int value) {
    nex_cmd("%s.val=%d", obj, value);
}

int nex_event(uint8_t *id, uint8_t *value) {
    static uint8_t frame[4];
    static uint8_t frame_len = 0;
    static uint8_t ff_count = 0;

    if (id == NULL || value == NULL) return 0;

    while (rx_tail != rx_head) {
        uint8_t byte = rx_buf[rx_tail];
        rx_tail = (uint8_t)((rx_tail + 1U) % RX_SIZE);

        if (byte == 0xFF) {
            if (++ff_count == 3U) {
                uint8_t length = frame_len;
                ff_count = 0;
                frame_len = 0;

                if (length >= 1U) {
                    *id = frame[0];
                    *value = (length >= 2U) ? frame[1] : 0U;
                    return 1;
                }
            }
        } else {
            ff_count = 0;
            if (frame_len < sizeof(frame)) {
                frame[frame_len++] = byte;
            } else {
                frame_len = 0;
            }
        }
    }

    return 0;
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
	if (c == '\n') uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = (uint8_t)c;
	return 0;
}

int uart_getchar(FILE *stream) {
	(void)stream;
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

void nex_text(const char *obj, const char *fmt, ...) {
    char buf[LINE_MAX];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printf("%s: %s\n", obj, buf);
}

void nex_cmd(const char *fmt, ...) {
    char buf[LINE_MAX];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printf("%s\n", buf);
}

void nex_val(const char *obj, int value) {
    printf("%s.val=%d\n", obj, value);
}

int nex_event(uint8_t *id, uint8_t *value) {
    (void)id;
    (void)value;
    return 0;
}

void io_redirect(void) {
    stdout = &uart_output;
    stdin = &uart_input;
}

#endif
