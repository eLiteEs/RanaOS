#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

const uint32_t SECTOR_SIZE = 512;
const uint32_t MAX_FILES   = 32;
const uint32_t TABLE_START = 1;
const uint32_t DATA_START  = 5;

struct FileEntry {
    char     name[16];     // Nombre del archivo (sin carpetas)
    uint32_t size;         // Tamaño en bytes
    uint32_t offset;       // Sector donde empieza
};

void trimName(const std::string& path, char* out) {
    auto pos = path.find_last_of("/\\");
    std::string name = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    std::strncpy(out, name.c_str(), 15);
    out[15] = '\0';
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " disco.img archivo1 archivo2 ...\n";
        return 1;
    }

    const char* outDisk = argv[1];
    std::vector<std::string> files;
    for (int i = 2; i < argc; ++i) files.push_back(argv[i]);

    std::ofstream disk(outDisk, std::ios::binary | std::ios::trunc);
    if (!disk.is_open()) {
        std::cerr << "Error creando disco: " << outDisk << "\n";
        return 1;
    }

    // Reservar tamaño base (256 sectores)
    disk.seekp(256 * SECTOR_SIZE - 1);
    disk.write("", 1); // expandimos el archivo

    // Crear tabla de archivos
    FileEntry table[MAX_FILES] = {};
    uint32_t currentSector = DATA_START;

    for (size_t i = 0; i < files.size() && i < MAX_FILES; ++i) {
        std::ifstream in(files[i], std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "No se pudo abrir: " << files[i] << "\n";
            continue;
        }

        in.seekg(0, std::ios::end);
        uint32_t size = in.tellg();
        in.seekg(0, std::ios::beg);

        trimName(files[i], table[i].name);
        table[i].size = size;
        table[i].offset = currentSector;

        std::vector<char> data(size);
        in.read(data.data(), size);

        disk.seekp(currentSector * SECTOR_SIZE);
        disk.write(data.data(), size);

        currentSector += (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        std::cout << "Archivo '" << table[i].name << "' -> sector " << table[i].offset << ", " << size << " bytes\n";
    }

    // Escribir tabla
    disk.seekp(TABLE_START * SECTOR_SIZE);
    disk.write(reinterpret_cast<char*>(table), sizeof(table));

    std::cout << "Sistema Fatnenuphar creado en '" << outDisk << "'\n";
    return 0;
}

