#include "pwm_servo4.h"

void servo4_init(void)
{
	DDRD |= (1 << PD3);

	// Activar canal B sobre Timer2 ya configurado por servo3_init()
	// COM2B1=1 -> clear OC2B subida, set bajada (no inversor)
	TCCR2A |= (1 << COM2B1);

	OCR2B = S4_MID; // Posición inicial: 90°
}

void servo4_write(uint8_t val)
{
	if (val < S4_MIN) val = S4_MIN;
	if (val > S4_MAX) val = S4_MAX;
	OCR2B = val;
}

void servo4_set_from_adc(uint8_t adc_val)
{
	uint8_t pulso = S4_MIN +
	(uint8_t)((uint16_t)adc_val * (S4_MAX - S4_MIN) / 255U);
	OCR2B = pulso;
}
void servo4_write_deg(uint8_t grados)
{
	if (grados > 180) grados = 180;
	uint8_t ocr = S4_MIN + (uint8_t)((uint16_t)grados * (S4_MAX - S4_MIN) / 180U);
	OCR2B = ocr;
}