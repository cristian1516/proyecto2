#include "pwm_servo1.h"

void servo1_init(void)
{
	DDRB |= (1 << PB1);

	// Timer1 Modo 14: Fast PWM TOP=ICR1
	// COM1A1=1 -> clear OC1A en match, set en TOP
	// CS11=1   -> prescaler 8
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13)  | (1 << WGM12) | (1 << CS11);

	ICR1  = S1_TOP;
	OCR1A = S1_MID; // Posición inicial: 90°
}

void servo1_write(uint16_t val)
{
	if (val < S1_MIN) val = S1_MIN;
	if (val > S1_MAX) val = S1_MAX;
	OCR1A = val;
}

void servo1_set_from_adc(uint8_t adc_val)
{
	uint16_t pulso = S1_MIN +
	((uint32_t)adc_val * (S1_MAX - S1_MIN)) / 255U;
	OCR1A = pulso;
}
// Escribe posición en grados (0-180) directamente
void servo1_write_deg(uint8_t grados)
{
	if (grados > 180) grados = 180;
	uint16_t ocr = S1_MIN + ((uint32_t)grados * (S1_MAX - S1_MIN)) / 180U;
	OCR1A = ocr;
}