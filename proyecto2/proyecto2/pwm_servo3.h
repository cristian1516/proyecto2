#ifndef PWM_SERVO3_H
#define PWM_SERVO3_H

#include <avr/io.h>

// ============================================================
//  pwm_servo3.h  -  Servo 3: Codo  (ATmega328PB)
//
//  Timer2 canal A  |  8-bit  |  ~50 Hz
//  Salida: OC2A = PB3 = D11
//
//  Timer2 es de 8 bits. Para obtener 50 Hz usamos Modo 5
//  (Fast PWM, TOP = OCR2A) con prescaler 1024:
//
//  Tick = 1 / (16MHz / 1024) = 64 us
//  Para período de 20 ms: TOP = (20000 / 64) - 1 = 311
//  Pero OCR2A es 8 bits (max 255) -> no alcanza con este método
//
//  Solución: prescaler 256
//  Tick = 1 / (16MHz / 256) = 16 us
//  TOP = (20000 / 16) - 1 = 1249 -> tampoco cabe en 8 bits
//
//  Solución real: usar Modo 1 (Phase Correct PWM, TOP=0xFF)
//  con prescaler 1024:
//  f_PWM = 16MHz / (2 * 1024 * 256) = ~30.5 Hz
//  Suficiente para servos (aceptan 40-200 Hz)
//
//  Rango de pulso con prescaler 1024, Phase Correct:
//  OCR2A = (pulso_us / (2 * 64)) = pulso_us / 128
//  0.5 ms = 500 us -> OCR2A =  4 (min)
//  1.5 ms = 1500 us -> OCR2A = 12 (mid)
//  2.4 ms = 2400 us -> OCR2A = 19 (max)
// ============================================================

#define S3_MIN    4U   //  0°
#define S3_MID   12U   // 90°
#define S3_MAX   19U   // 180°

void servo3_init(void);
void servo3_set_from_adc(uint8_t adc_val);
void servo3_write(uint8_t val);

#endif
void servo3_write_deg(uint8_t grados);