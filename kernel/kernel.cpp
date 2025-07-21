//
// eLite Systems RanaOS
// (c) 2025 Blas Fernández
// 32-bit OS coded with C++
// ns como puede ser que esto funcione
//

// Incluir funciones de otros archivos
#include "Console.h" // I/O Texto
#include "io.h" // I/O Archivos
#include "floppy.h" // cosas de disquetes
#include "fatnenuphar.h" // Filesystem de FatNenuphar
#include <stdint.h> // Algo de C++ que si esta en freestanding
#include "parrot.cpp" // Frames de un pajaro
#include "ata_detect.cpp" // Cosas para detectar discos
#include "fat32.h" // Funciones para el Fat32
#include "multiboot.h" // Funciones para cosas del GRUB
#include "Graphics.h" // Graficos en VGA 13h

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

// Cosas para las fechas
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
		Console::write("  di || disks >> Get the available disks.\n");
        Console::write("  shutdwn || shutdown >> Power off the computer.\n");
        Console::write("      ... /y >> Power off the computer without asking.\n");
        Console::write("  reboot >> Reboot the computer.\n");
	    Console::write("      ... /y >> Reboot the computer without asking.\n");
        Console::write("  jogo >> Play a game on graphical mode.\n");
        Console::write("  3d >> Shooow a rotating 3d cube on the screen.\n");
    } else if(!strcmp(s, "version")) {
		Console::write("eLite Systems RanaOS beta 2\nLicensed with GNU GPL v3.\n");
	} else if(!strcmp(substr(s, 0, 5), "echo ")) {
		Console::write(substr(s, 5));
		Console::putChar('\n');
	} else if(!strcmp(s, "clear") || !strcmp(s, "cls")) {
		Console::clearScreen();
	} else if(!strcmp(s, "time")) {
		Console::println(getHour(), ":", format_wth_0(getMinute()), ".", getSecond());
	} else if(!strcmp(s, "date")) {
		Console::println(getDay(), "/", format_wth_0(getMonth()), "/", getYear());
	} else if(!strcmp(s, "di") || !strcmp(s, "disks")) {
        	detect_disks();
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
    } else if(!strcmp(s, "ls")) {
        ls_fat32('C');
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

        gfx_init();
        gfx_clear_screen(COLOR_BLUE);

        // Some variables
        int x = 10;
        int y = 10;

        int fruitX = 40;
        int fruitY = 40;

        int points = 0;

        while(true)
        {
            gfx_clear_screen(COLOR_BLUE); // Clear screen

            gfx_fill_rect(x, y, 10, 10, 0x04); // Draw player

            gfx_fill_circle(fruitX + 5, fruitY + 5, 5, 0x06); // Draw fruit

            // Draw points
            for (int i = 0; i < points; i++) 
            {
                gfx_fill_rect(2 + 4 * i, 2, 2, 4, 0x06);
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
    } else {
		Console::write("Unknown Command. Use 'help' to get a list of commands.\n");
	}
}

void enable_cursor_blink() {
    outb(0x3D4, 0x0A);
    uint8_t val = inb(0x3D5);
    val &= ~(1 << 5); // clear bit 5 = habilitar parpadeo
    outb(0x3D5, val);
}

extern "C" void kmain(uint32_t magic, multiboot_info_t* mbi2) {
    // Arch-like startup
    Console::clearScreen();
    Console::println("Starting RanaOS...");

    // Init PIT
    Console::write("[ TASK ]    ", 3);
    Console::println("Starting PIT...");
    Console::write("[ INFO ]    ", 9);
    Console::println("Using default configuration (1 tick = 1 millisecond)");

    pit_init(1000);

    Console::write("[ SUCCESS ] ", 2);
    Console::println("PIT Started successfully!");

    // Save CPU speed
    Console::write("[ TASK ]    ", 3);
    Console::println("Gathering CPU speed...");

    g_cycles_per_ms = measure_cpu_frequency();

    Console::write("[ SUCCESS ] ", 2);
    Console::println("CPU Speed saved succesfully!");
    
    // Enable cursor
    Console::write("[ TASK ]    ", VGA_COLOR_CYAN);
    Console::println("Enabling cursor...");

    enable_cursor_blink();
    Console::enable_cursor(0, 15);
    Console::set_cursor(0);

    Console::write("[ SUCCESS ] ", 2);
    Console::println("Cursor enabled!");

    // Check Multiboot header
    Console::write("[ TASK ]    ", VGA_COLOR_CYAN);
    Console::println("Checking GRUB multiboot header magic number...");
    if (magic != 0x2BADB002) {  // Magic number for Multiboot-compliant bootloader
        Console::write("[ FATAL ERROR ] ", VGA_COLOR_RED);
        Console::println("Magic number not maching expected value, can't load RanaOS.");
        Console::println("Press any key to exit RanaOS...");

        bool b = false;
        Console::getKey(b);
        return;
    }
    Console::write("[ SUCCESS ] ", VGA_COLOR_GREEN);
    Console::println("Magic nuber value correct");

    // Move mbi to local mbi
    Console::write("[ TASK ]    ", VGA_COLOR_CYAN);
    Console::println("Move MBI information...");

    mbi = mbi2;

    Console::write("[ SUCCESS ] ", 9);
    Console::println("MBI information moved successfully");

    Console::write("[ INFO ]    ", VGA_COLOR_LIGHT_BLUE);
    Console::println("Entering RanaOS in 1500 ms");

    wait_ms(1500);

	// OG Loading
    Console::clearScreen();
    Console::write(" eLite      ", 0, 4);
    wait_ms(500);
    Console::write(" Systems    ", 0, 2);
    wait_ms(500);
    Console::write(" RanaOS     ", 0, 3);
    wait_ms(500);
    Console::write(" beta 2     ", 0, 14);

    wait_ms(250);

	Console::write("\n\n");

	Console::write("eLite Systems ");
	Console::write(" RanaOS beta 2 ", 0, 2);

	Console::write("\n\n");

    Console::println(substr(get_weekday_name(), 0, 3), ", ", format_wth_0(getDay()), "/", format_wth_0(getMonth()), "/", getYear(), "\n");

	Console::println("Use \"help\" for getting a list of commands.\n");

	while (1) {
		Console::write("> ");
		char* s = Console::readLine(linebuf, sizeof(linebuf));
		
		runcommand(s, true);		
	}
}

