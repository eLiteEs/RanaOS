#ifndef DATE_H
#define DATE_H

#include <cstdint>
#include "io.h" // Funciones de entrada/salida

uint8_t getSecond();
uint8_t getMinute();
uint8_t getHour();
uint8_t getDay();
uint8_t getMonth();
uint8_t getYear();

const char* get_weekday_name();

#endif // DATE_H