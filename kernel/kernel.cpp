#include "Console.h"
#include "io.h"

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

		Console::putChar('\n');
	} else if(!strcmp(s, "version")) {
		Console::write("RanaOS alpha 4\nLicensed with GNU GPL v3.\n");
	} else if(!strcmp(substr(s, 0, 5), "echo ")) {
		Console::write(substr(s, 5));
		Console::putChar('\n');
	} else if(!strcmp(s, "clear") || !strcmp(s, "cls")) {
		Console::clearScreen();
	} else if(!strcmp(s, "time")) {
		Console::println(getHour(), ":", getMinute(), "\n");
	} else if(!strcmp(s, "date")) {
		Console::println(getDay(), "/", getMinute(), "/", getYear(), "\n");
	} else {
		Console::write("Unknown Command. Use 'help' to get a list of commands.\n");
	}
}

static char linebuf[256];

extern "C" void kmain() {
	Console::clearScreen();
	Console::write("RanaOS alpha 4\nUse 'help' for getting a list of commands.\n");

	Console::println(getHour(), ":", getMinute(), "  ", getDay(), "/", getMonth(), "/", getYear(), "\n");

	while (1) {
		Console::write("$- ");
		char* s = Console::readLine(linebuf, sizeof(linebuf));
		
		runcommand(s);		
	}
}

