#ifndef PWM_SERVO2_H
#define PWM_SERVO2_H

#include <avr/io.h>

// ============================================================
//  pwm_servo2.h  -  Servo 2: Hombro  (ATmega328PB)
//
//  Timer1 canal B  |  16-bit  |  50 Hz exacto
//  Salida: OC1B = PB2 = D10
//
//  Comparte ICR1 con servo1 — pwm_servo1_init() debe ir primero
//
//  S2_MIN = 1000  ->  0.5 ms  ->    0°
//  S2_MID = 2900  ->  1.45 ms ->   90°
//  S2_MAX = 4800  ->  2.4 ms  ->  180°
// ============================================================

#define S2_MIN   1000U
#define S2_MID   2900U
#define S2_MAX   4800U

// IMPORTANTE: llamar servo1_init() antes que servo2_init()
void servo2_init(void);
void servo2_set_from_adc(uint8_t adc_val);
void servo2_write(uint16_t val);

#endif
void servo2_write_deg(uint8_t grados);