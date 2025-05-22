#include "Console.h"
#include "io.h"
#include "floppy.h"
#include "fatnenuphar.h"
#include <stdint.h>
#include "parrot.cpp"

#define DISK_SIZE_BYTES (128 * 1024)
#define MAX_FILE_SIZE   2048

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

void set_cursor(uint8_t row, uint8_t col) {
    uint16_t pos = row * 80 + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

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

uint64_t g_cycles_per_ms = 0;

void delay_ms(uint32_t ms) {
    uint64_t start = rdtsc();
    uint64_t end = start + g_cycles_per_ms * ms;
    while (rdtsc() < end);
}

uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

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

uint32_t measure_cpu_frequency() {
	pit_init(100); // 100 Hz = 10 ms tick

	uint32_t start = (uint32_t)rdtsc();
	pit_wait_ticks(10);
	uint32_t end = (uint32_t)rdtsc();

	uint32_t cycles = end - start;
	return cycles / 100;
}


int keyboard_key_available() {
    return inb(0x64) & 1;
}

uint8_t keyboard_read_scancode() {
    return inb(0x60);
}

bool was_c_pressed() {
    if (!keyboard_key_available())
        return false;

    uint8_t sc = keyboard_read_scancode();

    // Ignora tecla liberada (bit 7 = 1)
    if (sc & 0x80) return false;

    // Código 0x2E = tecla 'C' (scancode set 1)
    return sc == 0x2E;
}

static constexpr int MAX_FILE_ENTRIES = FN_DIR_ENTRIES;

// ———————— buffers únicos ————————
static uint8_t diskImageBuffer[DISK_SIZE_BYTES];
static char    fileReadBuffer[MAX_FILE_SIZE];

// ———————— Función de lectura genérica ————————
char* readFile(const char* filename) {
    int bytesRead = fn_read_file(filename, fileReadBuffer, MAX_FILE_SIZE);
    return (bytesRead > 0) ? fileReadBuffer : nullptr;
}

void listFilesOnFloppy() {
    // 1) Leer todo el floppy al buffer
    if (!floppy_read_image(DISK_SIZE_BYTES / FLOPPY_SECTOR_SIZE,
                           diskImageBuffer))
    {
        Console::write("Error leyendo floppy\n");
        return;
    }

    // 2) Inicializar Fatnenuphar
    fn_init(diskImageBuffer, DISK_SIZE_BYTES);

    // 3) Obtener la lista de entradas
    FN_FileEntry entries[MAX_FILE_ENTRIES];
    int count = fn_list_files(entries, MAX_FILE_ENTRIES);

    // 4) Imprimir
    Console::write("Archivos en Fatnenuphar:\n");
    for (int i = 0; i < count; ++i) {
        Console::write("  - ");
        Console::write(entries[i].filename);
        Console::write("\n");
    }
    if (count == 0) {
        Console::write("  (ninguno)\n");
    }
}


#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

uint8_t read_cmos(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

uint8_t getSecond() {
    return bcd_to_bin(read_cmos(0x00));
}

uint8_t getMinute() {
    return bcd_to_bin(read_cmos(0x02));
}

uint8_t getHour() {
    return bcd_to_bin(read_cmos(0x04));
}

uint8_t getDay() {
    return bcd_to_bin(read_cmos(0x07));
}

uint8_t getMonth() {
    return bcd_to_bin(read_cmos(0x08));
}

uint8_t getYear() {
    return bcd_to_bin(read_cmos(0x09)); // Solo últimos dos dígitos
}
const char* get_weekday_name() {
    int day = getDay();
    int month = getMonth();
    int year = getYear() + 2000;
    static const char* weekdays[] = {
        "Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri"
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

void runcommand(char* s) {	
	if(!strcmp(s, "help")) {
		Console::write("RanaOS - Help\n");
		Console::write("  help >> Show this list.\n");
		Console::write("  version >> Show the version of the current release of RanaOS.\n");
		Console::write("  echo [text] >> Print text to the screen.\n");
		Console::write("  clear || cls >> Clear the screen.\n");
		Console::write("  time >> Show current time.\n");
		Console::write("  date >> Show current date.\n");
		Console::write("  parrot >> Dancing parrot animation from ascii.live.\n");

		Console::putChar('\n');
	} else if(!strcmp(s, "version")) {
		Console::write("RanaOS beta 2\nLicensed with GNU GPL v3.\n");
	} else if(!strcmp(substr(s, 0, 5), "echo ")) {
		Console::write(substr(s, 5));
		Console::putChar('\n');
	} else if(!strcmp(s, "clear") || !strcmp(s, "cls")) {
		Console::clearScreen();
	} else if(!strcmp(s, "time")) {
		Console::println(getHour(), ":", getMinute(), "\n");
	} else if(!strcmp(s, "date")) {
		Console::println(getDay(), "/", getMonth(), "/", getYear(), "\n");
	} else if(!strcmp(substr(s, 0, 5), "read ")) {	
		fn_init(diskImageBuffer, DISK_SIZE_BYTES);	
	
    		char* content = readFile(substr(s, 5));
    		if (!content) {
        		Console::write("Archivo no encontrado o error leyendo.\n");
    		} else {
        		Console::write("Contenido:\n");
        		Console::write(content);
		}
	} else if(!strcmp(s, "ls") || !strcmp(s, "dir")) {
		// 1) Leer todos los sectores del floppy
    		if (!floppy_read_image(DISK_SIZE_BYTES / FLOPPY_SECTOR_SIZE,
                           diskImageBuffer))
    		{
        		Console::write("Error leyendo floppy\n");
        		return;
    		}

    		// 2) Inicializar Fatnenuphar usando el buffer y su tamaño
    		fn_init(diskImageBuffer, DISK_SIZE_BYTES);


		listFilesOnFloppy();
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
        		if (parrot[i] == NULL)
            			i = 0;

			for(int j = 0; j < 10000; j++) {	}
    		}
	} else {
		Console::write("Unknown Command. Use 'help' to get a list of commands.\n");
		Console::putChar('\n');
	}
}

static char linebuf[256];

void enable_cursor_blink() {
    outb(0x3D4, 0x0A);
    uint8_t val = inb(0x3D5);
    val &= ~(1 << 5); // clear bit 5 = habilitar parpadeo
    outb(0x3D5, val);
}

extern "C" void kmain() {
	Console::clearScreen();
	Console::write("RanaOS beta 2", 0, 2);

	Console::putChar('\n');

	Console::write("Starting PIT (1 tick = 1 millisecond)... ");

	pit_init(1000);	

	Console::println("PIT Started successfully!");

	Console::write("Reading CPU frequency... ");

	g_cycles_per_ms = measure_cpu_frequency();

	Console::println((int)measure_cpu_frequency(), " GHz");

	Console::println("Enabling cursor...");
	enable_cursor_blink();
	Console::enable_cursor(0, 15);
	Console::set_cursor(0);

	Console::println(getHour(), ":", getMinute(), "  ", get_weekday_name(), ", ", getDay(), "/", getMonth(), "/", getYear(), "\n");

	Console::println("Use 'help' for getting a list of commands.\n");

	while (1) {
		Console::write("$- ");
		char* s = Console::readLine(linebuf, sizeof(linebuf));
		
		runcommand(s);		
	}
}

