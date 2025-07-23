// kernel/math.hpp
#pragma once

#include "math.hpp"
#include <stdint.h>

/**
 * Convierte una cadena hexadecimal a decimal
 * @param hex Cadena hexadecimal (ej: "1A3F")
 * @return Valor decimal (ej: 6719). Retorna 0 si hay error.
 */
uint32_t hex_to_dec(const char* hex) {
    uint32_t result = 0;
    
    if (!hex) return 0; // Verificación de seguridad
    
    // Saltar prefijo "0x" si existe
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    
    while (*hex) {
        char c = *hex++;
        uint8_t value;
        
        // Convertir caracter a valor numérico
        if (c >= '0' && c <= '9') {
            value = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            value = 10 + (c - 'A');
        } else if (c >= 'a' && c <= 'f') {
            value = 10 + (c - 'a');
        } else {
            return 0; // Caracter inválido
        }
        
        // Desplazar y sumar
        result = (result << 4) | value;
    }
    
    return result;
}