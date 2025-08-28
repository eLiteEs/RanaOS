#include "kernel.cpp"

extern "C" {

const char* rread_file_from_meta(const char* name) {
    return read_file_from_meta(const_cast<char*>(name));
}

void run_elf(char* name) {
    run_program(name); 
}

}
