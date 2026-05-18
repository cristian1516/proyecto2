#include "eeprom_brazo.h"
#include <avr/eeprom.h>

// ============================================================
//  eeprom_brazo.c  -  Implementación EEPROM  (ATmega328PB)
// ============================================================

uint8_t eeprom_count(void)
{
    uint8_t count = eeprom_read_byte((uint8_t*)EEPROM_COUNT_ADDR);
    // 0xFF significa que la EEPROM nunca fue escrita
    if (count == 0xFF) return 0;
    return count;
}

void eeprom_guardar(uint8_t base, uint8_t hombro,
                    uint8_t codo, uint8_t garra)
{
    uint8_t count = eeprom_count();
    if (count >= MAX_POSICIONES) return;

    uint16_t addr = EEPROM_DATA_START + ((uint16_t)count * 4);
    eeprom_update_byte((uint8_t*)(addr + 0), base);
    eeprom_update_byte((uint8_t*)(addr + 1), hombro);
    eeprom_update_byte((uint8_t*)(addr + 2), codo);
    eeprom_update_byte((uint8_t*)(addr + 3), garra);
    eeprom_update_byte((uint8_t*)EEPROM_COUNT_ADDR, count + 1);
}

void eeprom_leer(uint8_t idx,
                 uint8_t* base, uint8_t* hombro,
                 uint8_t* codo, uint8_t* garra)
{
    uint16_t addr = EEPROM_DATA_START + ((uint16_t)idx * 4);
    *base   = eeprom_read_byte((uint8_t*)(addr + 0));
    *hombro = eeprom_read_byte((uint8_t*)(addr + 1));
    *codo   = eeprom_read_byte((uint8_t*)(addr + 2));
    *garra  = eeprom_read_byte((uint8_t*)(addr + 3));
}

void eeprom_reset(void)
{
    eeprom_update_byte((uint8_t*)EEPROM_COUNT_ADDR, 0);
}
