#ifndef EEPROM_BRAZO_H
#define EEPROM_BRAZO_H

#include <avr/io.h>

// ============================================================
//  eeprom_brazo.h  -  Librería EEPROM  (ATmega328PB)
//
//  Layout en EEPROM:
//    Addr 0       -> número de posiciones grabadas (uint8_t)
//    Addr 1+      -> base, hombro, codo, garra (1 byte c/u)
//    Máximo 32 posiciones = 128 bytes de datos
//
//  Uso:
//    eeprom_guardar(base, hombro, codo, garra)  -> guarda posición
//    eeprom_leer(i, &base, &hombro, &codo, &garra) -> lee posición i
//    eeprom_count() -> retorna número de posiciones guardadas
//    eeprom_reset()  -> borra todas las posiciones
// ============================================================

#define EEPROM_COUNT_ADDR   0
#define EEPROM_DATA_START   1
#define MAX_POSICIONES      32

void    eeprom_guardar(uint8_t base, uint8_t hombro,
                       uint8_t codo, uint8_t garra);

void    eeprom_leer(uint8_t idx,
                    uint8_t* base, uint8_t* hombro,
                    uint8_t* codo, uint8_t* garra);

uint8_t eeprom_count(void);
void    eeprom_reset(void);

#endif
