// kernel/math.hpp
#pragma once

#include <stdint.h>

/**
 * Convierte una cadena hexadecimal a decimal
 * @param hex Cadena hexadecimal (ej: "1A3F")
 * @return Valor decimal (ej: 6719). Retorna 0 si hay error.
 */
uint32_t hex_to_dec(const char* hex);