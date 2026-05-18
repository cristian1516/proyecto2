#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//  MODOS:
//    M1 = Manual  -> pots controlan servos  | LED D6 ON
//    M2 = EEPROM  -> reproduce grabaciones  | LED D7 ON
//    M3 = UART    -> dashboard Adafruit     | D6 + D7 ON
//
//  COMANDOS UART desde Python/Adafruit:
//    M1\n / M2\n / M3\n  -> cambiar modo
//    B<grados>\n          -> mover base    (0-180)
//    H<grados>\n          -> mover hombro  (0-180)
//    C<grados>\n          -> mover codo    (0-180)
//    R<grados>\n          -> mover garra   (0-180)
//    G\n                  -> guardar posición actual en EEPROM
//
//  CONEXIONES:
//    Pot 1 -> A0  |  Servo Base   -> D9  (OC1A, Timer1 16-bit)
//    Pot 2 -> A1  |  Servo Hombro -> D10 (OC1B, Timer1 16-bit)
//    Pot 3 -> A2  |  Servo Codo   -> D11 (OC2A, Timer2 8-bit)
//    Pot 4 -> A3  |  Servo Garra  -> D3  (OC2B, Timer2 8-bit)
//    LED Manual  -> D6 (PD6)
//    LED EEPROM  -> D7 (PD7)
// ============================================================

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "pwm_servo1.h"
#include "pwm_servo2.h"
#include "pwm_servo3.h"
#include "pwm_servo4.h"
#include "uart.h"
#include "eeprom_brazo.h"

// ??????????????????????????????????????????????
//  CONSTANTES
// ??????????????????????????????????????????????

#define MODO_MANUAL   1
#define MODO_EEPROM   2
#define MODO_UART     3

#define LED_MANUAL    PD6
#define LED_EEPROM    PD7

#define EEPROM_DELAY_MS  1000   // Tiempo entre posiciones al reproducir

// ??????????????????????????????????????????????
//  VARIABLES GLOBALES
// ??????????????????????????????????????????????

uint8_t modo     = MODO_MANUAL;

// Posición actual de cada servo en grados (0-180)
// Se actualiza siempre que se mueve un servo
uint8_t pos_base   = 90;
uint8_t pos_hombro = 90;
uint8_t pos_codo   = 90;
uint8_t pos_garra  = 90;

// ??????????????????????????????????????????????
//  GPIO  —  LEDs de modo
// ??????????????????????????????????????????????

void gpio_init(void)
{
	DDRD |= (1 << LED_MANUAL) | (1 << LED_EEPROM);
	PORTD &= ~((1 << LED_MANUAL) | (1 << LED_EEPROM));
}

void leds_set_modo(uint8_t m)
{
	PORTD &= ~((1 << LED_MANUAL) | (1 << LED_EEPROM));
	if (m == MODO_MANUAL) PORTD |=  (1 << LED_MANUAL);
	if (m == MODO_EEPROM) PORTD |=  (1 << LED_EEPROM);
	if (m == MODO_UART)   PORTD |=  (1 << LED_MANUAL) | (1 << LED_EEPROM);
}

// ??????????????????????????????????????????????
//  ADC
// ??????????????????????????????????????????????

void adc_init(void)
{
	DIDR0 |= (1 << ADC0D) | (1 << ADC1D) |
	(1 << ADC2D) | (1 << ADC3D);
	ADMUX  = (1 << REFS0) | (1 << ADLAR);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// Conversión de descarte inicial
	ADMUX  = (1 << REFS0) | (1 << ADLAR) | 0;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
}

uint8_t adc_read(uint8_t canal)
{
	ADMUX = (1 << REFS0) | (1 << ADLAR) | (canal & 0x07);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADCH;
}

// Convierte ADC 0-255 a grados 0-180
uint8_t adc_to_degrees(uint8_t val)
{
	return (uint8_t)((uint16_t)val * 180 / 255);
}

// ??????????????????????????????????????????????
//  FILTRO DE PROMEDIO MÓVIL + HISTÉRESIS
//
//  Promedio móvil: promedia N lecturas para eliminar
//  el ruido de alta frecuencia del ADC.
//
//  Histéresis: solo actualiza el servo si el cambio
//  en grados supera el umbral. Evita que pequeñas
//  fluctuaciones hagan vibrar el servo.
// ??????????????????????????????????????????????

#define FILTRO_MUESTRAS   8     // Lecturas a promediar por canal
#define HISTERESIS_GRADOS 2     // Cambio mínimo en grados para mover

uint8_t adc_filtrado(uint8_t canal)
{
	uint16_t suma = 0;
	for (uint8_t i = 0; i < FILTRO_MUESTRAS; i++)
	suma += adc_read(canal);
	return (uint8_t)(suma / FILTRO_MUESTRAS);
}

uint8_t aplicar_histeresis(uint8_t nuevo, uint8_t anterior)
{
	uint8_t diff = (nuevo > anterior) ? (nuevo - anterior) : (anterior - nuevo);
	return (diff >= HISTERESIS_GRADOS) ? nuevo : anterior;
}


//  MOVER SERVOS
//  Recibe grados (0-180) y llama directamente a
//  servo_write_deg que mapea a OCR correctamente.
//  Sin conversiones intermedias.
// ??????????????????????????????????????????????

void mover_base(uint8_t g)
{
	if (g > 180) g = 180;
	pos_base = g;
	servo1_write_deg(g);
}

void mover_hombro(uint8_t g)
{
	if (g > 180) g = 180;
	pos_hombro = g;
	servo2_write_deg(g);
}

void mover_codo(uint8_t g)
{
	if (g > 180) g = 180;
	pos_codo = g;
	servo3_write_deg(g);
}

void mover_garra(uint8_t g)
{
	if (g > 180) g = 180;
	pos_garra = g;
	servo4_write_deg(g);
}


//  MODO EEPROM  —  Reproducir posiciones


void reproducir_eeprom(void)
{
	uint8_t count = eeprom_count();

	if (count == 0)
	{
		uart_send_string("Sin posiciones grabadas\r\n");
		return;
	}

	char buf[8];
	uart_send_string("Reproduciendo ");
	itoa(count, buf, 10);
	uart_send_string(buf);
	uart_send_string(" posiciones\r\n");

	for (uint8_t i = 0; i < count; i++)
	{
		// Salir si llega un cambio de modo durante la reproducción
		char cmd[32];
		while (uart_readline(cmd, sizeof(cmd)))
		{
			if (cmd[0] == 'M')
			{
				uint8_t m = (uint8_t)atoi(&cmd[1]);
				if (m >= 1 && m <= 3)
				{
					modo = m;
					leds_set_modo(modo);
					return;
				}
			}
		}

		uint8_t b, h, c, g;
		eeprom_leer(i, &b, &h, &c, &g);

		mover_base(b);
		mover_hombro(h);
		mover_codo(c);
		mover_garra(g);

		uart_send_string("Pos ");
		itoa(i + 1, buf, 10);
		uart_send_string(buf);
		uart_send_string("\r\n");

		// Parpadeo LED EEPROM en cada paso
		PORTD &= ~(1 << LED_EEPROM);
		_delay_ms(150);
		PORTD |=  (1 << LED_EEPROM);

		_delay_ms(EEPROM_DELAY_MS);
	}

	uart_send_string("Reproduccion terminada\r\n");
}


//  PARSEAR COMANDOS UART
//
//  Formato recibido desde Python:
//    "M1", "M2", "M3"     -> cambio de modo
//    "B90", "H45"         -> mover servo con número
//    "G"                  -> guardar posición (G sin número)
//    "B90", "C30", etc.   -> mover servo en modo UART


void procesar_comando(const char* cmd)
{
	if (strlen(cmd) == 0) return;

	char tipo = cmd[0];

	// Debug: confirmar comando recibido
	uart_send_string("CMD: ");
	uart_send_string(cmd);
	uart_send_string("\r\n");

	// Ignorar release values "0" que envía Adafruit al soltar botones
	if (strcmp(cmd, "0") == 0) return;

	// ?? Cambio de modo: M1, M2, M3 ??
	if (tipo == 'M')
	{
		uint8_t m = (uint8_t)atoi(&cmd[1]);
		if (m >= 1 && m <= 3)
		{
			modo = m;
			leds_set_modo(modo);
			const char* nombres[] = {"", "Manual", "EEPROM", "UART"};
			uart_send_string("Modo: ");
			uart_send_string(nombres[modo]);
			uart_send_string("\r\n");
		}
		return;
	}

	// Guardar posición: "G" sin número
	if (tipo == 'G' && strlen(cmd) == 1)
	{
		eeprom_guardar(pos_base, pos_hombro, pos_codo, pos_garra);
		char buf[8];
		uart_send_string("Guardada pos ");
		itoa(eeprom_count(), buf, 10);
		uart_send_string(buf);
		uart_send_string("\r\n");
		return;
	}

	// Mover servo: funciona en modo UART y también en Manual
	//    para que los comandos lleguen aunque el modo cambie con delay 
	uint8_t grados = (uint8_t)atoi(&cmd[1]);
	if (grados > 180) grados = 180;

	if      (tipo == 'B') mover_base(grados);
	else if (tipo == 'H') mover_hombro(grados);
	else if (tipo == 'C') mover_codo(grados);
	else if (tipo == 'R') mover_garra(grados);
}


//  MAIN


int main(void)
{
	gpio_init();
	adc_init();
	uart_init();

	// Orden obligatorio: canal A antes que canal B en cada timer
	servo1_init();   // Timer1 A
	servo2_init();   // Timer1 B (sobre Timer1 ya configurado)
	servo3_init();   // Timer2 A
	servo4_init();   // Timer2 B (sobre Timer2 ya configurado)

	// Posición inicial: todos al centro
	mover_base(90);
	mover_hombro(90);
	mover_codo(90);
	mover_garra(90);

	leds_set_modo(MODO_MANUAL);
	uart_send_string("Brazo listo - Modo Manual\r\n");

	char cmd_buf[32];

	while (1)
	{
		// Procesar TODOS los comandos disponibles en el buffer UART
		// antes de continuar con el modo actual
		while (uart_readline(cmd_buf, sizeof(cmd_buf)))
		procesar_comando(cmd_buf);

		// ?? Modo Manual: pots controlan servos ??
		if (modo == MODO_MANUAL)
		{
			static uint8_t prev_base   = 90;
			static uint8_t prev_hombro = 90;
			static uint8_t prev_codo   = 90;
			static uint8_t prev_garra  = 90;

			// ADC 0-255 -> grados 0-180
			uint8_t g_base   = adc_to_degrees(adc_filtrado(0));
			uint8_t g_hombro = adc_to_degrees(adc_filtrado(1));
			uint8_t g_codo   = adc_to_degrees(adc_filtrado(2));
			uint8_t g_garra  = adc_to_degrees(adc_filtrado(3));

			g_base   = aplicar_histeresis(g_base,   prev_base);
			g_hombro = aplicar_histeresis(g_hombro, prev_hombro);
			g_codo   = aplicar_histeresis(g_codo,   prev_codo);
			g_garra  = aplicar_histeresis(g_garra,  prev_garra);

			if (g_base   != prev_base)   { mover_base(g_base);     prev_base   = g_base;   }
			if (g_hombro != prev_hombro) { mover_hombro(g_hombro); prev_hombro = g_hombro; }
			if (g_codo   != prev_codo)   { mover_codo(g_codo);     prev_codo   = g_codo;   }
			if (g_garra  != prev_garra)  { mover_garra(g_garra);   prev_garra  = g_garra;  }

			_delay_ms(5);   // 5ms: revisar UART más frecuentemente
		}

		// ?? Modo EEPROM: reproducir posiciones grabadas ??
		else if (modo == MODO_EEPROM)
		{
			reproducir_eeprom();
			// Al terminar (sin interrupción) volver a manual
			if (modo == MODO_EEPROM)
			{
				modo = MODO_MANUAL;
				leds_set_modo(MODO_MANUAL);
				uart_send_string("Volviendo a Manual\r\n");
			}
		}

		// ?? Modo UART: movimientos vienen por serial ??
		else if (modo == MODO_UART)
		{
			_delay_ms(5);   // 5ms: revisar UART frecuentemente
		}
	}

	return 0;
}