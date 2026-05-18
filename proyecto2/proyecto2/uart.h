#ifndef UART_H
#define UART_H

#include <avr/io.h>

// ============================================================
//  uart.h  -  Librería UART  (ATmega328PB)
//
//  9600 baud, 8N1, 16MHz
//  UBRR = (16000000 / (16 * 9600)) - 1 = 103
//
//  TX -> D1 (PD1)
//  RX -> D0 (PD0)
//
//  Protocolo de comandos recibidos:
//    M1\n        -> modo Manual
//    M2\n        -> modo EEPROM
//    M3\n        -> modo UART
//    B<0-180>\n  -> mover servo base
//    H<0-180>\n  -> mover servo hombro
//    C<0-180>\n  -> mover servo codo
//    G<0-180>\n  -> mover servo garra
//    G\n         -> guardar posición en EEPROM
// ============================================================

void uart_init(void);
void uart_send_char(char c);
void uart_send_string(const char* s);

// Lee una línea completa terminada en '\n', no bloqueante.
// Retorna 1 si hay línea lista en buf, 0 si aún no.
uint8_t uart_readline(char* buf, uint8_t maxlen);

#endif
