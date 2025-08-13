// Cosas para las fechas
#include "date.h"

#include <cstdint>
#include "io.h" // Funciones de entrada/salida

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// Leer datos del CMOS
uint8_t read_cmos(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

// Transformar bdc a binario
uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

// Funciones para las horas
uint8_t getSecond() {
    return bcd_to_bin(read_cmos(0x00));
}

uint8_t getMinute() {
    return bcd_to_bin(read_cmos(0x02));
}

uint8_t getHour() {
    return bcd_to_bin(read_cmos(0x04));
}

// Funciones para las fechas
uint8_t getDay() {
    return bcd_to_bin(read_cmos(0x07));
}

uint8_t getMonth() {
    return bcd_to_bin(read_cmos(0x08));
}

uint8_t getYear() {
    return bcd_to_bin(read_cmos(0x09)); // Solo últimos dos dígitos
}

// Funcion para obtener el nombre del dia de hoy
const char* get_weekday_name() {
    int day = getDay();
    int month = getMonth();
    int year = getYear() + 2000;
    static const char* weekdays[] = {
        "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
	};

    if (month < 3) {
        month += 12;
        year -= 1;
    }

    int k = year % 100;
    int j = year / 100;

    int h = (day + 13*(month + 1)/5 + k + k/4 + j/4 + 5*j) % 7;

    return weekdays[h];
}