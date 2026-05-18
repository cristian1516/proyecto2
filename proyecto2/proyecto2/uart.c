#include "uart.h"
#include <string.h>

// ============================================================
//  uart.c  -  Implementación UART  (ATmega328PB)
// ============================================================

void uart_init(void)
{
	// UBRR = (F_CPU / (16 * baud)) - 1 = 103 para 9600 baud a 16MHz
	// Error de reloj: 0.2% — completamente estable
	uint16_t ubrr = 103;
	UBRR0H = (ubrr >> 8);
	UBRR0L =  ubrr;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send_char(char c)
{
	// Esperar hasta que el buffer de transmisión esté libre
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

void uart_send_string(const char* s)
{
	while (*s)
	uart_send_char(*s++);
}

uint8_t uart_readline(char* buf, uint8_t maxlen)
{
	// Buffer interno estático: persiste entre llamadas
	static char    rbuf[32];
	static uint8_t idx = 0;

	// Leer todos los bytes disponibles sin bloquear
	while (UCSR0A & (1 << RXC0))
	{
		char c = UDR0;

		if (c == '\n' || c == '\r')
		{
			// Fin de línea: si hay contenido, copiar y retornar
			if (idx > 0)
			{
				rbuf[idx] = '\0';
				strncpy(buf, rbuf, maxlen - 1);
				buf[maxlen - 1] = '\0';
				idx = 0;
				return 1;
			}
			// Si llegó \r seguido de \n, ignorar el segundo
		}
		else if (idx < (uint8_t)(sizeof(rbuf) - 1))
		{
			rbuf[idx++] = c;
		}
		// Si el buffer interno se llena sin \n, descartar
		else
		{
			idx = 0;
		}
	}

	return 0;  // Línea aún no completa
}