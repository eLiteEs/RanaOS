//
// eLite Systems RanaOS
// (c) 2025 Blas Fernández
// 32-bit OS coded with C++
// ns como puede ser que esto funcione
//

// Incluir funciones de otros archivos
#include "Console.h" // I/O Texto
#include "io.h" // I/O Archivos
#include <stdint.h> // Algo de C++ que si esta en freestanding
#include "parrot.cpp" // Frames de un pajaro
#include "multiboot.h" // Funciones para cosas del GRUB
#include "Graphics.h" // Graficos en VGA 13h
#include "string.h" // ns
#include "math.hpp" // Math operations
#include "string.hpp"
#include "idt.h"
#include "vgraphics.h"
#include "Debug.h" // Debugging functions
#include "date.h"

// Algo del filesystem
#define DISK_SIZE_BYTES (128 * 1024)
#define MAX_FILE_SIZE   2048

// Cosas del PIT
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

// Colorines
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_WHITE         15

void wait_ms(uint32_t ms) {
    if (ms == 0) return;

    // Each iteration is limited to 54.925 ms (maximum divisor = 65535)
    while (ms > 0) {
        uint32_t chunk = (ms > 54) ? 54 : ms;
        ms -= chunk;

        uint16_t divisor = (uint16_t)(1193182 / 1000 * chunk); // = 1193 * chunk

        // Set PIT channel 0 to mode 0 (one-shot), binary counting
        outb(PIT_COMMAND, 0b00110100); // channel 0, access lobyte/hibyte, mode 0

        // Load divisor
        outb(PIT_CHANNEL0, divisor & 0xFF);        // low byte
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF); // high byte

        // Wait until the countdown is done (OUT == 1)
        while (true) {
            outb(PIT_COMMAND, 0xE2); // latch status of channel 0
            uint8_t status = inb(PIT_CHANNEL0);
            if (status & (1 << 7)) break; // OUT = 1, finished
        }
    }
}

// Cosas del parrot
uint64_t rdtsc();

// Posicionar el cursor en row y col (esta en Console.h tmb)
void set_cursor(uint8_t row, uint8_t col) {
    uint16_t pos = row * 80 + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Mas cosas del PIT
void pit_init(uint32_t frequency) {
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);

    // Enviar comando: canal 0, modo 3 (square wave), acceso low/high
    outb(PIT_COMMAND, 0x36);

    // Enviar divisor en dos partes (low, high)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));        // Low byte
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF)); // High byte
}

void pit_set_frequency(uint32_t freq) {
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / freq);
    outb(PIT_COMMAND, 0x36); // canal 0, modo 3 (square wave)
    outb(PIT_CHANNEL0, divisor & 0xFF);       // low byte
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF); // high byte
}

uint64_t g_cycles_per_ms = 0; // Ciclos de la CPU

// Delay usando la CPU
void delay_ms(uint32_t ms) {
    uint64_t start = rdtsc();
    uint64_t end = start + g_cycles_per_ms * ms;
    while (rdtsc() < end);
}

// en verdad no se que es esto
uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

// esto tampoco se que es
void io_wait() {
    outb(0x80, 0);
}

// Espera una interrupción del PIT (método simple sin IDT):
void pit_wait_ticks(uint32_t ticks) {
    for (uint32_t i = 0; i < ticks; ++i) {
        // Canal 0 cuenta hasta 0 → bit 7 del puerto 0x61 se vuelve 1 (Hack)
        while (!(inb(0x61) & 0x20)) {
            io_wait();
        }
        // Clear bit para esperar siguiente tick
        outb(0x61, inb(0x61) & ~0x20);
    }
}

// Para calcular la frequencia de la CPU (va raro)
uint32_t measure_cpu_frequency() {
	pit_init(100); // 100 Hz = 10 ms tick

	uint32_t start = (uint32_t)rdtsc();
	pit_wait_ticks(10);
	uint32_t end = (uint32_t)rdtsc();

	uint32_t cycles = end - start;
	return cycles / 100;
}

// Funciones que estan redifinidas
int keyboard_key_available() {
    return inb(0x64) & 1;
}

uint8_t keyboard_read_scancode() {
    return inb(0x60);
}

// Funcion para comprobar si c fue pulsada
bool was_c_pressed() {
    if (!keyboard_key_available())
        return false;

    uint8_t sc = keyboard_read_scancode();

    // Ignora tecla liberada (bit 7 = 1)
    if (sc & 0x80) return false;

    // Código 0x2E = tecla 'C' (scancode set 1)
    return sc == 0x2E;
}

// Funciones útiles para no usar libc
int strcmp(const char* a, const char* b) {
	while (*a && *b) {
        	if (*a != *b) {
            	    return (*a - *b);
        	}
        	a++;
        	b++;
	}
	return (*a - *b);
}

char* substr(const char* str, int start, int length = -1) {
    static char buffer[256]; // Ajusta el tamaño según tus necesidades
    int i = 0;

    if (length == -1) {
        // Copia desde `start` hasta el final del string
        while (str[start + i] != '\0' && i < 255) {
            buffer[i] = str[start + i];
            i++;
        }
    } else {
        // Copia desde `start` hasta `start + length`
        while (i < length && str[start + i] != '\0') {
            buffer[i] = str[start + i];
            i++;
        }
    }

    buffer[i] = '\0';
    return buffer;
}

bool isNumber(const char* str) {
    if (!str || *str == '\0') return false;

    // Saltar espacios
    while (*str == ' ' || *str == '\t') ++str;

    // Manejar signo
    if (*str == '-' || *str == '+') ++str;

    // Debe haber al menos un dígito
    if (*str < '0' || *str > '9') return false;

    while (*str) {
        if (*str < '0' || *str > '9') return false;
        ++str;
    }

    return true;
}
int stoi(const char* str) {
    int result = 0;
    int sign = 1;

    // Saltar espacios en blanco
    while (*str == ' ' || *str == '\t') {
        ++str;
    }

    // Manejar signo negativo
    if (*str == '-') {
        sign = -1;
        ++str;
    } else if (*str == '+') {
        ++str;
    }

    // Convertir dígitos
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        ++str;
    }

    return result * sign;
}

void wait(int secs) {
	int now = getSecond();
	while(getSecond() != now + secs) {}
}

char* int_to_str(int value) {
    static char buffer[12];
    char* ptr = buffer + sizeof(buffer) - 1;
    bool neg = value < 0;
    *ptr = '\0';
    unsigned int u = neg ? -value : value;
    do {
        *--ptr = '0' + (u % 10);
        u /= 10;
    } while (u);
    if (neg) *--ptr = '-';
    return ptr;
}

char* concat(const char* a, const char* b) {
    static char buffer[256];
    char* p = buffer;
    while (*a) *p++ = *a++;
    while (*b) *p++ = *b++;
    *p = 0;
    return buffer;
}

char* format_wth_0(int i) {
    if(i <= 9) {
        return concat("0", int_to_str(i));
    } else {
        return int_to_str(i);
    } 

    return 0;
}

// Random things
static uint32_t rand_seed = 1;

// Generador LCG básico
uint32_t rand_lcg() {
    // Parámetros del generador LCG (usados en glibc)
    rand_seed = (1103515245 * rand_seed + 12345) & 0x7FFFFFFF;
    return rand_seed;
}

// Función para establecer la semilla
void srand_lcg(uint32_t seed) {
    rand_seed = seed;
}

// Función que genera un número aleatorio entre min y max (inclusive)
int32_t rand_range(int32_t min, int32_t max) {
    if (min > max) {
        // Intercambiar si min es mayor que max
        int32_t temp = min;
        min = max;
        max = temp;
    }
    
    // Calcular el rango
    uint32_t range = max - min + 1;
    
    // Generar número aleatorio en el rango [0, range-1]
    uint32_t rand_val = rand_lcg() % range;
    
    // Ajustar al rango [min, max] y devolver
    return min + rand_val;
}

static char linebuf[256]; // Algo para la consola
multiboot_info_t* mbi; // var global Mulitboot Info
bool shownModulesExplanation = false; // Flag para saber si se explico los modulos de GRUB en la sesion actual

// Codigos de flechas
#define KEY_LEFT       0x4B
#define KEY_RIGHT      0x4D
#define KEY_UP         0x48
#define KEY_DOWN       0x50 
#define KEY_BACKSPACE  0x0E
#define KEY_ENTER      0x1C

// Matematicas confusas
static float sinf(float x) {
    // Aproximación con serie de Taylor
    float result = x;
    float term = x;
    float x2 = x * x;
    for (int n = 1; n < 5; n++) {
        term *= -x2 / ((2*n) * (2*n+1));
        result += term;
    }
    return result;
}

static float cosf(float x) {
    // Aproximación con serie de Taylor
    float result = 1.0f;
    float term = 1.0f;
    float x2 = x * x;
    for (int n = 1; n < 5; n++) {
        term *= -x2 / ((2*n-1) * (2*n));
        result += term;
    }
    return result;
}

// Cosas para el cubo que gira
static float rotation_x = 0.0f;
static float rotation_y = 0.0f;
static float rotation_z = 0.0f;

// Función para rotar un vértice alrededor del origen
static void rotate_vertex(Vec3* vertex, float angle_x, float angle_y, float angle_z) {
    // Rotación en X
    float cos_x = cosf(angle_x);
    float sin_x = sinf(angle_x);
    float y = vertex->y * cos_x - vertex->z * sin_x;
    float z = vertex->y * sin_x + vertex->z * cos_x;
    vertex->y = y;
    vertex->z = z;

    // Rotación en Y
    float cos_y = cosf(angle_y);
    float sin_y = sinf(angle_y);
    float x = vertex->x * cos_y + vertex->z * sin_y;
    z = -vertex->x * sin_y + vertex->z * cos_y;
    vertex->x = x;
    vertex->z = z;

    // Rotación en Z (opcional)
    float cos_z = cosf(angle_z);
    float sin_z = sinf(angle_z);
    x = vertex->x * cos_z - vertex->y * sin_z;
    y = vertex->x * sin_z + vertex->y * cos_z;
    vertex->x = x;
    vertex->y = y;
}

// Definir los 12 triángulos de un cubo (2 por cara)
static Triangle cube[12] = {
    // Cara frontal (rojo)
    {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, 0x04},
    {{-1, -1, 1}, {1, 1, 1}, {-1, 1, 1}, 0x04},
    // Cara derecha (verde)
    {{1, -1, 1}, {1, -1, -1}, {1, 1, -1}, 0x02},
    {{1, -1, 1}, {1, 1, -1}, {1, 1, 1}, 0x02},
    // Cara trasera (azul)
    {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, 0x01},
    {{-1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, 0x01},
    // Cara izquierda (amarillo)
    {{-1, -1, 1}, {-1, -1, -1}, {-1, 1, -1}, 0x0E},
    {{-1, -1, 1}, {-1, 1, -1}, {-1, 1, 1}, 0x0E},
    // Cara superior (magenta)
    {{-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, 0x05},
    {{-1, 1, 1}, {1, 1, -1}, {-1, 1, -1}, 0x05},
    // Cara inferior (cyan)
    {{-1, -1, 1}, {1, -1, 1}, {1, -1, -1}, 0x03},
    {{-1, -1, 1}, {1, -1, -1}, {-1, -1, -1}, 0x03}
};

// Función para dibujar el cubo rotando
void draw_rotating_cube() {
    while (1) {
        gfx_clear_screen(0x00);  // Borrar pantalla (fondo negro)

        // Rotar todos los vértices del cubo
        for (int i = 0; i < 12; i++) {
            rotate_vertex(&cube[i].v0, rotation_x, rotation_y, rotation_z);
            rotate_vertex(&cube[i].v1, rotation_x, rotation_y, rotation_z);
            rotate_vertex(&cube[i].v2, rotation_x, rotation_y, rotation_z);
        }

        // Dibujar todos los triángulos
        gfx_draw_mesh(cube, 12);

        // Actualizar ángulos de rotación
        rotation_x += 0.02f;
        rotation_y += 0.01f;

        wait_ms(100);
    }
}

/**
 * Lee una cadena desde una dirección de memoria
 * @param start_addr Dirección de inicio (ej: 0x00100000)
 * @param max_len Longitud máxima a leer (evita desbordamientos)
 * @return Puntero a cadena terminada en null (o nullptr si falla)
 */
const char* read_string_from_memory(uint32_t start_addr, size_t max_len = 1024) {
    // 1. Verificación básica de dirección
    if (start_addr < 0x100000 || start_addr > 0xC0000000) {
        Console::println("Error: Dirección de memoria inválida");
        return nullptr;
    }

    // 2. Buffer estático para seguridad (evita allocs dinámicos)
    static char buffer[1025]; // +1 para el null terminator
    const char* src = (const char*)start_addr;

    // 3. Copia caracteres hasta encontrar null o exceder max_len
    size_t i;
    for (i = 0; i < max_len && i < sizeof(buffer) - 1; i++) {
        // Verifica cada byte antes de copiar
        if ((uint32_t)(src + i) >= 0xC0000000) break; // Evita accesos peligrosos
        
        buffer[i] = src[i];
        if (src[i] == '\0') break; // Fin de cadena
    }

    // 4. Asegura terminación nula
    buffer[i] = '\0';

    // 5. Verifica si la cadena es válida
    if (i == 0 || buffer[0] == '\0') {
        return nullptr;
    }

    return buffer;
}

/**
 * Lee una cadena desde memoria con control preciso del tamaño máximo
 * @param start_addr Dirección de memoria inicial
 * @param max_bytes Máximo de bytes a leer (0 para automático hasta null terminator)
 * @return Puntero a la cadena leída o NULL en error
 */
const char* read_large_memory_string(uint32_t start_addr, size_t max_bytes) {
    // Límites de seguridad ajustados para tu SO
    const uint32_t MIN_ADDR = 0x100000;
    const uint32_t MAX_ADDR = 0xC0000000;
    const size_t STATIC_BUFFER_SIZE = 1.5 * 1024 * 1024; // 1.5MB buffer estático
    
    // 1. Verificación básica de dirección
    if (start_addr < MIN_ADDR || start_addr >= MAX_ADDR) {
        Console::println("Error: Dirección de memoria inválida");
        return nullptr;
    }

    // 2. Determinar tamaño máximo real a leer
    size_t safe_max_bytes;
    if (max_bytes == 0) {
        // Modo automático: leer hasta encontrar null terminator
        safe_max_bytes = STATIC_BUFFER_SIZE - 1;
    } else {
        // Asegurar que no excedamos nuestro buffer estático
        safe_max_bytes = (max_bytes < STATIC_BUFFER_SIZE) ? max_bytes : STATIC_BUFFER_SIZE - 1;
        
        // Verificar que no intentamos leer más allá de la memoria permitida
        if (start_addr + safe_max_bytes >= MAX_ADDR) {
            safe_max_bytes = MAX_ADDR - start_addr - 1;
        }
    }

    // 3. Buffer estático ampliado (1.5MB)
    static char buffer[STATIC_BUFFER_SIZE];
    
    // 4. Copia segura byte por byte
    size_t bytes_read = 0;
    const char* src = (const char*)start_addr;
    
    while (bytes_read < safe_max_bytes) {
        // Verificación de acceso para cada byte (conservador)
        if ((uint32_t)(src + bytes_read) >= MAX_ADDR) {
            break;
        }
        
        buffer[bytes_read] = src[bytes_read];
        
        // Terminar si encontramos null terminator (excepto si max_bytes > 0)
        if (buffer[bytes_read] == '\0' && max_bytes == 0) {
            break;
        }
        
        bytes_read++;
    }

    // 5. Garantizar terminación nula
    buffer[bytes_read < safe_max_bytes ? bytes_read : safe_max_bytes] = '\0';
    
    return (bytes_read > 0) ? buffer : nullptr;
}

// Función auxiliar para verificar regiones de memoria (implementar según tu SO)
bool memory_region_is_safe(uint32_t addr, uint32_t size) {
    // Implementación básica - ajustar según tu sistema
    return (addr >= 0x100000 && (addr + size) <= 0xC0000000);
}

const char* read_file_from_meta(char* name) {
    multiboot_module_t* mods = (multiboot_module_t*)mbi->mods_addr;

    for (uint32_t i = 0; i < mbi->mods_count; i++) 
    {
        int start = mods[i].mod_start;

        // Check if it isn't a valid module
        if(strcmp(read_string_from_memory(start, 2), "n:")) {
            continue;
        }

        // Check if the module has the name we're searching
        if(strcmp(read_string_from_memory(start, 2 + String::strlen(name)), concat("n:", name))) {
            continue;
        }

        int lengthStart = 5 + String::strlen(name) + start;
        int length;

        // Get the size of the file
        int j = 0;
        bool finished = false;
        char* rawResult = "";

        while(!finished) {
            const char* c = read_string_from_memory(lengthStart + j, 1); // Get the char

            if(!strcmp(c, "b")) {
                finished = true;
            } else {
                if(isNumber(c)) {
                    char* temp = rawResult;
                    rawResult = concat(temp, c);
                } else {
                    finished = true;
                }
            }

            j++;
        }

        // Parse the file length
        length = stoi(rawResult) + 1;

        // Get the content start
        int contentStart = lengthStart + String::strlen(rawResult) + 2;

        // Return the actual content
        return read_large_memory_string(contentStart, length);
    }

    Console::write("File not found", VGA_COLOR_RED);
    return nullptr;
}

extern "C" void start_so();

// Ejecutar comandos
void runcommand(char* s, bool auth) {	
	if(!strcmp(s, "help")) {
		Console::write("RanaOS - Help\n");
		Console::write("  help >> Show this list.\n");
		Console::write("  version >> Show the version of the current release of RanaOS.\n");
		Console::write("  echo [text] >> Print text to the screen.\n");
		Console::write("  clear || cls >> Clear the screen.\n");
		Console::write("  time >> Show current time.\n");
		Console::write("  date >> Show current date.\n");
		Console::write("  parrot >> Dancing parrot animation from ascii.live.\n");
		Console::write("  day >> Get the weekday name.\n");
        Console::write("  shutdwn || shutdown >> Power off the computer.\n");
        Console::write("      ... /y >> Power off the computer without asking.\n");
        Console::write("  reboot >> Reboot the computer.\n");
	    Console::write("      ... /y >> Reboot the computer without asking.\n");
        Console::write("  jogo >> Play a game on graphical mode.\n");
        Console::write("  3d >> Shooow a rotating 3d cube on the screen.\n");
        Console::write("  auth >> Prints yes if the command was runned authenticated.\n");
        Console::write("  hex [hexadecimal] >> Shows the decimal value of a hexadecimal string.\n");
        Console::write("  read [filename] >> Read the contents of a module in a better way.\n");
        Console::write("  start >> Start Os in graphical mode.\n");
    } else if(!strcmp(s, "version")) {
		Console::write("eLite Systems RanaOS beta 3\nLicensed with GNU GPL v3.\n");
	} else if(!strcmp(substr(s, 0, 5), "echo ")) {
		Console::println(substr(s, 5));
	} else if(!strcmp(s, "clear") || !strcmp(s, "cls")) {
		Console::clearScreen();
	} else if(!strcmp(s, "time")) {
		Console::println(getHour(), ":", format_wth_0(getMinute()), ".", getSecond());
	} else if(!strcmp(s, "date")) {
		Console::println(getDay(), "/", format_wth_0(getMonth()), "/", getYear());
	} else if(!strcmp(s, "di") || !strcmp(s, "disks")) {
        //detect_disks();
        Console::println("Removed temporalily.");
    } else if(!strcmp(s, "parrot")) {
    	int i = 0;

		while(true) {
    		Console::clearScreen();
        	Console::println(parrot[i]);
        	pit_wait_ticks(1000);

        	if (was_c_pressed()) {
    			break;
            }

        	i++;
        	if (parrot[i] == NULL) {
           		i = 0;
			} else {
                wait_ms(50);
            }
		}	
	} else if(!strcmp(s, "day")) {
		Console::println(get_weekday_name());
	} else if(!strcmp(s, "shutdwn") || !strcmp(s, "shutdown")) {
        Console::write("Are you sure that you want to power off the computer? (y=yes, else=no): ");
        bool tr = false;
        char answer = Console::getKey(tr);

        if(answer == 0x15) {
            outw(0xB004, 0x2000);

            outw(0x604, 0x2000);

            outw(0x4004, 0x3400);

            for (;;) {
                __asm__ __volatile__("hlt");
            }
        }
    } else if(!strcmp(s, "reboot")) {
        Console::write("Are you sure that you want to reboot the computer? (y=yes, else=no): ");
        bool tr = false;
        char answer = Console::getKey(tr);
        
        if(answer == 0x15) {
            while (inb(0x64) & 0x02);
            outb(0x64, 0xFE);
        }
    } else if(!strcmp(substr(s, 0, 5), "wait ")) {
        char* time = substr(s, 5);

        if(isNumber(time)) {
            wait_ms(stoi(time));
        } else {
            Console::write("The introduced delay isn't a number.\n");
        }
    } else if(!strcmp(s, "shutdwn /y") || !strcmp(s, "shutdown /y")) {
        if(auth) {
            outw(0xB004, 0x2000);

            outw(0x604, 0x2000);

            outw(0x4004, 0x3400);

            for (;;) {
                __asm__ __volatile__("hlt");
            }
        } else {
            Console::println("No permission for powering off the computer.");
        }
    } else if(!strcmp(s, "reboot /y")) {
        if(auth) {
            while (inb(0x64) & 0x02);
            outb(0x64, 0xFE);
        } else {
            Console::println("No permission for rebooting the computer.");
        } 
    } else if(!strcmp(s, "jogo") /* aka boludez */) {
        Console::clearScreen();

        VGraphics::fillRect(0, 0, 1920, 1080, 0x0000ff); // Clear screen

        // Some variables
        int x = 10;
        int y = 10;

        int fruitX = 40;
        int fruitY = 40;

        int points = 0;

        while(true)
        {
            VGraphics::fillRect(0, 0, 1920, 1080, 0x0000ff); // Clear screen

            VGraphics::fillRect(x, y, 10, 10, 0xff0000); // Draw player

            VGraphics::fillRect(fruitX, fruitY, 10, 10, 0x00ff00); // Draw point

            // Draw points
            for (int i = 0; i < points; i++) 
            {
                VGraphics::fillRect(i * 2 + 1, 2, 1, 2, 0x00ff00); // Draw point
            }

            // Move the player on key
            bool tr = false;
            char answer = Console::getKey(tr);

            if(answer == KEY_RIGHT)
            {
                if(x + 10 != 310)
                {
                    x += 10;
                }
                
            } else if(answer == KEY_LEFT) 
            {
                if(x - 10 != 0)
                {
                    x -= 10;
                }
            } else if(answer == KEY_UP)
            {
                if(y - 10 != 0)
                {
                    y -= 10;
                }

            } else if(answer == KEY_DOWN) 
            {
                if(y + 10 != 190)
                {
                    y += 10;
                }
            } else 
            {
                break;
            }

            // Chekc collision between player and fruit
            if(x == fruitX && y == fruitY) {
                points++;
                fruitX = rand_range(10, 30) * 10;
                fruitY = rand_range(10, 18) * 10;
            }
        }

        // Exit the game
        while (inb(0x64) & 0x02);
        outb(0x64, 0xFE);
    } else if(!strcmp(s, "3d")) {    
        gfx_init();  // Configurar modo VGA/gráfico
        float aspect = 120 / (float)80;
        gfx_set_projection(3.141592f / 3.0f, aspect, 0.1f, 100.0f);  // FOV=60°
        gfx_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);  // Cámara en (0,0,-5)
        draw_rotating_cube();  // Iniciar animación
    } else if(!strcmp(substr(s, 0, 5), "read ")) {
        char* name = substr(s, 5);

        Console::println(read_file_from_meta(name));
    } else if(!strcmp(s, "auth")) {
        if(auth) {
            Console::println("yes");
        } else {
            Console::println("no");
        }
    } else if(!strcmp(substr(s, 0, 4), "hex ")) {
        char* hex = substr(s, 4);

        Console::println("Decimal: ", (long long unsigned) hex_to_dec(hex));
        Console::println("Char: ", (char) ((long long unsigned) hex_to_dec(hex)));
    } else if(!strcmp(s, "image")) {
        const char* imageContent = read_file_from_meta("tux.pim");

        Console::clearScreen();

        gfx_init();
        gfx_clear_screen(COLOR_WHITE);

        gfx_put_image(0,0, imageContent);

        while(true) {}

    } else if(!strcmp(s, "start")) {
        start_so(); // Llamar a la función de inicio del SO
    } else {
	Console::write("Unknown Command. Use \"help\" to get a list of commands.\n");
    }
}

void enable_cursor_blink() {
    outb(0x3D4, 0x0A);
    uint8_t val = inb(0x3D5);
    val &= ~(1 << 5); // clear bit 5 = habilitar parpadeo
    outb(0x3D5, val);
}

void init_pic() {
    // ICW1: Inicialización
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // ICW2: Vector offset
    outb(0x21, 0x20); // IRQ 0-7 mapeado a 0x20-0x27
    outb(0xA1, 0x28); // IRQ 8-15 mapeado a 0x28-0x2F
    
    // ICW3: Conexión master-slave
    outb(0x21, 0x04); // Slave en IRQ2
    outb(0xA1, 0x02);
    
    // ICW4: Modo 8086
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Enmascarar todas las interrupciones
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

extern "C" void kmain(uint32_t magic, multiboot_info_t* mbi2) {
    // First of all, check if the magic number is correct
    if (magic != 0x36D76289) {  // Magic number for Multiboot-compliant bootloader
        Console::write("[FATAL ERROR] ", VGA_COLOR_RED);
        Console::println("Magic number not maching expected value, can't load RanaOS.");
        Console::println("Press any key to exit RanaOS...");

        bool b = false;
        Console::getKey(b);
        return;
    }
    mbi = mbi2; // Set global MBI pointer

    FBInfo fb = {};
    uint8_t* ptr = (uint8_t*)mbi2;
    ptr += 8; // skip total_size and reserved

    while (true) {
        uint32_t type = *(uint32_t*)(ptr);
        uint32_t size = *(uint32_t*)(ptr + 4);

        if (type == 0) break;

        if (type == 8) {
            fb.address = (uint32_t*)(*(uint64_t*)(ptr + 8));
            fb.pitch   = *(uint32_t*)(ptr + 16);
            fb.width   = *(uint32_t*)(ptr + 20);
            fb.height  = *(uint32_t*)(ptr + 24);
            fb.bpp     = *(uint8_t*)(ptr + 28);

            VGraphics::init(fb.width, fb.height, fb.pitch, fb.bpp, reinterpret_cast<uintptr_t>(fb.address));
            break;
        }

        ptr += (size + 7) & ~7;
    }

    Console::setGraphics(true); // Set graphics mode

    // Arch-like startup
    Console::clearScreen();
    Console::println("Starting RanaOS...");

    // Start Debug serial port COM1
    Console::setColor((uint32_t)0x4287f5);
    Console::write("[TASK]    ");
    Console::setColor((uint32_t)0xffffff);
    Console::println("Starting Debug serial port COM1...");

    Debug::Init();
    Debug::Print("Debug port started.\n");

    Console::setColor((uint32_t)0x2bbf1b);
    Console::write("[SUCCESS] ", 2);
    Console::setColor((uint32_t)0xffffff);
    Console::println("Debug port started succesfully!");

    // Init PIT
    Console::setColor((uint32_t)0x4287f5);
    Console::write("[TASK]    ");
    Console::setColor((uint32_t)0xffffff);
    Console::println("Starting PIT...");

    Console::setColor((uint32_t)0x1b9ebf);
    Console::write("[INFO]    ", 9);
    Console::setColor((uint32_t)0xffffff);
    Console::println("Using default configuration (1 tick = 1 millisecond)");

    pit_init(1000);

    Console::setColor((uint32_t)0x2bbf1b);
    Console::write("[SUCCESS] ", 2);
    Console::setColor((uint32_t)0xffffff);
    Console::println("PIT Started successfully!");

    // Save CPU speed
    Console::setColor((uint32_t)0x4287f5);
    Console::write("[TASK]    ");
    Console::setColor((uint32_t)0xffffff);
    Console::println("Gathering CPU speed...");

    g_cycles_per_ms = measure_cpu_frequency();

    Console::setColor((uint32_t)0x2bbf1b);
    Console::write("[SUCCESS] ", 2);
    Console::setColor((uint32_t)0xffffff);
    Console::println("CPU Speed saved succesfully!");

    // Init IDT
    Console::setColor((uint32_t)0x4287f5);
    Console::write("[TASK]    ");
    Console::setColor((uint32_t)0xffffff);
    Console::println("Starting IDT...");

    idt_init();

    Console::setColor((uint32_t)0x2bbf1b);
    Console::write("[SUCCESS] ", 2);
    Console::setColor((uint32_t)0xffffff);
    Console::println("IDT started succesfully!");

    // Init PIC
    Console::setColor((uint32_t)0x4287f5);
    Console::write("[TASK]    ");
    Console::setColor((uint32_t)0xffffff);
    Console::println("Starting PIC...");

    init_pic();
    asm volatile("sti");

    Console::setColor((uint32_t)0x2bbf1b);
    Console::write("[SUCCESS] ", 2);
    Console::setColor((uint32_t)0xffffff);
    Console::println("PIC started succesfully!");

    // Move local mbi to global mbi
    Console::setColor((uint32_t)0x4287f5);
    Console::write("[TASK]    ");
    Console::setColor((uint32_t)0xffffff);
    Console::println("Move MBI information...");

    mbi = mbi2;

    Console::setColor((uint32_t)0x2bbf1b);
    Console::write("[SUCCESS] ", 2);
    Console::setColor((uint32_t)0xffffff);
    Console::println("MBI information moved successfully");

    Console::setColor((uint32_t)0x1b9ebf);
    Console::write("[INFO]    ", 9);
    Console::setColor((uint32_t)0xffffff);
    Console::println("Entering RanaOS in 1500 ms");

    wait_ms(1500);

	// OG Loading
    Console::clearScreen();
    Console::setColors((uint32_t)0x000000, (uint32_t)0xff0000); // White text on black background
    Console::write(" eLite      ");
    wait_ms(500);
    Console::setColors((uint32_t)0x000000, (uint32_t)0x00ff00);
    Console::write(" Systems    ");
    wait_ms(500);
    Console::setColors((uint32_t)0x000000, (uint32_t)0x4287f5);
    Console::write(" RanaOS     ");
    wait_ms(500);
    Console::setColors((uint32_t)0x000000, (uint32_t)0xffff00);
    Console::write(" beta 3     ");
    Console::setColors((uint32_t)0xffffff, (uint32_t)0x0); // Reset colors

    wait_ms(250);

	Console::write("\n\n");

	Console::write("eLite Systems ");
    
    Console::setbColor(0x00ff00); // Green background
    Console::setColor((uint32_t)0x000000); // Black text
	Console::write(" RanaOS beta 3 ", 0, 2);
    Console::setbColor(0x000000); // Reset background
    Console::setColor((uint32_t)0xffffff); // Reset text color


	Console::write("\n\n");

    Console::println(substr(get_weekday_name(), 0, 3), ", ", format_wth_0(getDay()), "/", format_wth_0(getMonth()), "/", getYear(), "\n");

	Console::println("Use \"help\" for getting a list of commands.\n");
    Console::println("To enter graphical mode, use the command \"start\".\n");

	while (1) {
		Console::write("> ");
        char* s = Console::readLine(linebuf, sizeof(linebuf));
		
		runcommand(s, true);		
	}
}
