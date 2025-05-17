#include "Console.h"

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
		Console::write("RanaOS alpha 4 - Help\n");
		Console::write("  help >> Show this list.\n");
		Console::write("  version >> Show the version of the current release of RanaOS.\n");
		Console::write("  echo [text] >> Print text to the console\n");
	} else if(!strcmp(s, "version")) {
		Console::write("RanaOS alpha 3\nLicensed with GNU GPL v3.\n");
	} else if(!strcmp(substr(s, 0, 5), "echo ")) {
		Console::write(substr(s, 5));
		Console::putChar('\n');
	} else {
		Console::write("Unknown Command. Use 'help' to get a list of commands.\n");
	}
}

static char linebuf[256];

extern "C" void kmain() {
	Console::clearScreen();
	Console::write("RanaOS alpha 4\nUse 'help' for getting a list of commands.\n");

	while (1) {
		Console::write("\n$- ");
		char* s = Console::readLine(linebuf, sizeof(linebuf));
		
		runcommand(s);		
	}
}

