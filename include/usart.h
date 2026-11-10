#ifndef USART_H_INCLUDED
#define USART_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

#ifndef BAUD
#define BAUD 9600UL
#endif

// 1: printf goes to the Nextion display
// 0: printf goes to the serial monitor
#define USE_NEXTION   0
#define NEX_LINES     3      // number of text boxes

// Sets the text of one display component by name:
//     nex_text("t0", "Hello world);
void nex_text(const char *obj, const char *fmt, ...);

#define t0_text(...)  nex_text("t0", __VA_ARGS__)
#define t1_text(...)  nex_text("t1", __VA_ARGS__)
#define t2_text(...)  nex_text("t2", __VA_ARGS__)

int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);

void uart_init(void);
void io_redirect(void);

#endif
