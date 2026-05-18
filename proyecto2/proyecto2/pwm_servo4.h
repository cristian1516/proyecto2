#ifndef PWM_SERVO4_H
#define PWM_SERVO4_H

#include <avr/io.h>

// ============================================================
//  pwm_servo4.h  -  Servo 4: Garra  (ATmega328PB)
//
//  Timer2 canal B  |  8-bit  |  ~30.5 Hz
//  Salida: OC2B = PD3 = D3
//
//  Comparte Timer2 con servo3 — servo3_init() debe ir primero
//
//  Mismo rango que servo3:
//  S4_MIN =  4  ->  0°
//  S4_MID = 12  -> 90°
//  S4_MAX = 19  -> 180°
// ============================================================

#define S4_MIN    4U
#define S4_MID   12U
#define S4_MAX   19U

// IMPORTANTE: llamar servo3_init() antes que servo4_init()
void servo4_init(void);
void servo4_set_from_adc(uint8_t adc_val);
void servo4_write(uint8_t val);

#endif
void servo4_write_deg(uint8_t grados);