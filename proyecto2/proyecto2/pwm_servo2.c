#include "pwm_servo2.h"

void servo2_init(void)
{
	DDRB |= (1 << PB2);

	// Activar canal B sobre Timer1 ya configurado por servo1_init()
	// COM1B1=1 -> clear OC1B en match, set en TOP
	TCCR1A |= (1 << COM1B1);

	OCR1B = S2_MID; // Posición inicial: 90°
}

void servo2_write(uint16_t val)
{
	if (val < S2_MIN) val = S2_MIN;
	if (val > S2_MAX) val = S2_MAX;
	OCR1B = val;
}

void servo2_set_from_adc(uint8_t adc_val)
{
	uint16_t pulso = S2_MIN +
	((uint32_t)adc_val * (S2_MAX - S2_MIN)) / 255U;
	OCR1B = pulso;
}
// Escribe posición en grados (0-180) directamente
void servo2_write_deg(uint8_t grados)
{
	if (grados > 180) grados = 180;
	uint16_t ocr = S2_MIN + ((uint32_t)grados * (S2_MAX - S2_MIN)) / 180U;
	OCR1B = ocr;
}