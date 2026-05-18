#ifndef PWM_SERVO1_H
#define PWM_SERVO1_H

#include <avr/io.h>

// ============================================================
//  pwm_servo1.h  -  Servo 1: Base  (ATmega328PB)
//
//  Timer1 canal A  |  16-bit  |  50 Hz exacto
//  Salida: OC1A = PB1 = D9
//
//  SG90 rango real:
//    S1_MIN = 1000  ->  0.5 ms  ->    0°
//    S1_MID = 2900  ->  1.45 ms ->   90°
//    S1_MAX = 4800  ->  2.4 ms  ->  180°
//
//  ICR1 = 39999  ->  (39999+1) * (8/16MHz) = 20 ms = 50 Hz
// ============================================================

#define S1_TOP  39999U
#define S1_MIN   1000U
#define S1_MID   2900U
#define S1_MAX   4800U

void servo1_init(void);
void servo1_set_from_adc(uint8_t adc_val);
void servo1_write(uint16_t val);

#endif
void servo1_write_deg(uint8_t grados);