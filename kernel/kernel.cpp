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

static char linebuf[256];

extern "C" void kmain() {
	Console::clearScreen();
	Console::write("RanaOS alpha 3\nUse 'help' for getting a list of commands.\n");

	while (1) {
		Console::write("\n$- ");
		char* s = Console::readLine(linebuf, sizeof(linebuf));
		
		if(!strcmp(s, "help")) {
			Console::write("RanaOS alpha 3 - Help\n");
			Console::write("  help >> Show this list.\n");
			Console::write("  version >> Show the version of the current release of RanaOS.\n");
		} else if(!strcmp(s, "version")) {
			Console::write("RanaOS alpha 3\nLicensed with GNU GPL v3.\n");
		} else {
			Console::write("Unknown Command. Use 'help' to get a list of commands.\n");
		}
	}
}

