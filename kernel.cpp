#include <stdint.h>
#include "Console.h"

extern "C" void kmain();
extern "C" void keyboard_handler();


extern "C" void kmain() {
    
	Console::clearScreen();
    
	Console::setColor(0x0F);
	
	Console::write("RanaOS - alpha 1");
	
	char* line = Console::readLine();

	Console::write("\nYou typed: ");
	Console::write(line);

	while (1) {}
}

