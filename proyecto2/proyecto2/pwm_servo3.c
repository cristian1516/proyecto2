#include "pwm_servo3.h"

void servo3_init(void)
{
	DDRB |= (1 << PB3);

	// Timer2 Modo 1: Phase Correct PWM, TOP = 0xFF
	// COM2A1=1 -> clear OC2A subida, set bajada (no inversor)
	// CS22=1 + CS21=1 + CS20=1 -> prescaler 1024
	// f_PWM = 16MHz / (2 * 1024 * 256) = ~30.5 Hz
	TCCR2A = (1 << COM2A1) | (1 << WGM20);
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

	OCR2A = S3_MID; // Posición inicial: 90°
}

void servo3_write(uint8_t val)
{
	if (val < S3_MIN) val = S3_MIN;
	if (val > S3_MAX) val = S3_MAX;
	OCR2A = val;
}

void servo3_set_from_adc(uint8_t adc_val)
{
	// Mapeo lineal 0-255 -> S3_MIN..S3_MAX
	uint8_t pulso = S3_MIN +
	(uint8_t)((uint16_t)adc_val * (S3_MAX - S3_MIN) / 255U);
	OCR2A = pulso;
}
void servo3_write_deg(uint8_t grados)
{
	if (grados > 180) grados = 180;
	uint8_t ocr = S3_MIN + (uint8_t)((uint16_t)grados * (S3_MAX - S3_MIN) / 180U);
	OCR2A = ocr;
}