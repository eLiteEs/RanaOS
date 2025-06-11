#include "fat32.h"
#include "Console.h"
#include "disk.h"
#include "io.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct __attribute__((packed)) BPB {
    uint8_t jmpBoot[3];
    uint8_t OEMName[8];
    uint16_t BytsPerSec;
    uint8_t SecPerClus;
    uint16_t RsvdSecCnt;
    uint8_t NumFATs;
    uint16_t RootEntCnt;
    uint16_t TotSec16;
    uint8_t Media;
    uint16_t FATSz16;
    uint16_t SecPerTrk;
    uint16_t NumHeads;
    uint32_t HiddSec;
    uint32_t TotSec32;

    // FAT32
    uint32_t FATSz32;
    uint16_t ExtFlags;
    uint16_t FSVer;
    uint32_t RootClus;
    uint16_t FSInfo;
    uint16_t BkBootSec;
    uint8_t Reserved[12];
    uint8_t DrvNum;
    uint8_t Reserved1;
    uint8_t BootSig;
    uint32_t VolID;
    uint8_t VolLab[11];
    uint8_t FilSysType[8];
};

struct __attribute__((packed)) DirEntry {
    char Name[11];
    uint8_t Attr;
    uint8_t NTRes;
    uint8_t CrtTimeTenth;
    uint16_t CrtTime;
    uint16_t CrtDate;
    uint16_t LstAccDate;
    uint16_t FstClusHI;
    uint16_t WrtTime;
    uint16_t WrtDate;
    uint16_t FstClusLO;
    uint32_t FileSize;
};

extern DiskInfo g_disks[4];
int g_disk_count = 2; // DEFINICIÓN correcta (sin extern)

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170
#define ATA_PRIMARY_CTRL 0x3F6
#define ATA_SECONDARY_CTRL 0x376

#define ATA_DATA        (ATA_PRIMARY_IO + 0)
#define ATA_ERROR       (ATA_PRIMARY_IO + 1)
#define ATA_SECTOR_CNT  (ATA_PRIMARY_IO + 2)
#define ATA_SECTOR_NUM  (ATA_PRIMARY_IO + 3)
#define ATA_CYL_LOW     (ATA_PRIMARY_IO + 4)
#define ATA_CYL_HIGH    (ATA_PRIMARY_IO + 5)
#define ATA_DRIVE_HEAD  (ATA_PRIMARY_IO + 6)
#define ATA_STATUS      (ATA_PRIMARY_IO + 7)
#define ATA_COMMAND     (ATA_PRIMARY_IO + 7)
#define ATA_ALT_STATUS  (ATA_PRIMARY_CTRL + 0)
#define ATA_DEVICE_CTRL (ATA_PRIMARY_CTRL + 0)

#define ATA_CMD_READ_SECTORS 0x20

#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08

#define TIMEOUT_LIMIT 1000000

bool wait_for_drive_ready(uint16_t io_base) {
    uint8_t status;
    int timeout = TIMEOUT_LIMIT;

    // Esperar hasta que BSY=0 y DRQ=1 o timeout
    do {
        status = inb(io_base + 7);
        if (--timeout == 0) return false;
    } while ((status & ATA_SR_BSY) || !(status & ATA_SR_DRQ));

    return true;
}

bool read_sector(uint8_t drive_index, uint32_t lba, uint8_t* buffer) {
    if (drive_index > 1) return false; // Solo disco primario o secundario

    uint16_t io_base = (drive_index == 0) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t ctrl_base = (drive_index == 0) ? ATA_PRIMARY_CTRL : ATA_SECONDARY_CTRL;

    // Esperar que el disco no esté ocupado con timeout
    int timeout = TIMEOUT_LIMIT;
    uint8_t status;
    do {
        status = inb(io_base + 7);
        if (--timeout == 0) return false;
    } while (status & ATA_SR_BSY);

    // Preparar para lectura de sector
    outb(ATA_SECTOR_CNT, 1);                          // sector count = 1
    outb(io_base + 2, (uint8_t)(lba & 0xFF));         // sector number (LBA bits 0-7)
    outb(io_base + 3, (uint8_t)((lba >> 8) & 0xFF));  // cylinder low (LBA bits 8-15)
    outb(io_base + 4, (uint8_t)((lba >> 16) & 0xFF)); // cylinder high (LBA bits 16-23)
    outb(io_base + 6, 0xE0 | ((drive_index & 1) << 4) | ((lba >> 24) & 0x0F)); // drive + LBA bits 24-27

    outb(io_base + 7, ATA_CMD_READ_SECTORS); // comando lectura

    if (!wait_for_drive_ready(io_base)) return false;

    insw(io_base + 0, buffer, 512 / 2); // leer 512 bytes

    return true;
}

uint32_t get_partition_start(uint8_t drive_index) {
    uint8_t mbr[512];
    if (!read_sector(drive_index, 0, mbr)) return 0;

    // Check MBR signature
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) return 0;

    // Get first partition entry (offset 0x1BE)
    uint8_t* pentry = mbr + 0x1BE;
    return *(uint32_t*)(pentry + 8); // Partition start LBA
}

void ls_fat32(char drive_letter) {
    uint8_t drive_index = drive_letter - 'C';
    if (drive_index >= g_disk_count) {
        Console::println("Unidad no detectada.");
        return;
    }

    uint8_t sector[512];
    uint32_t part_start = get_partition_start(drive_index);
    if (!part_start) {
        Console::println("No partition found");
        return;
    }

    // Read VBR from partition start
    if (!read_sector(drive_index, part_start, sector)) {
        Console::println("Failed to read VBR");
        return;
    }

    BPB* bpb = (BPB*)sector;

    // Validate VBR signature
    if (sector[510] != 0x55 || sector[511] != 0xAA) {
        Console::println("Invalid VBR signature");
        return;
    }

    uint32_t fatStart = bpb->RsvdSecCnt;
    uint32_t fatSz = bpb->FATSz32;
    uint32_t rootDirSectors = ((bpb->RootEntCnt * 32) + (bpb->BytsPerSec - 1)) / bpb->BytsPerSec;
    uint32_t firstDataSector = bpb->RsvdSecCnt + (bpb->NumFATs * bpb->FATSz32);
    uint32_t rootCluster = bpb->RootClus;
    uint32_t rootSector = ((bpb->RootClus - 2) * bpb->SecPerClus) + firstDataSector;
    
    rootSector += part_start; // Adjust for partition offset

    for (int s = 0; s < bpb->SecPerClus; ++s) {
        if (!read_sector(drive_index, rootSector + s, sector)) {
            Console::println("No se pudo leer el root.");
            return;
        }

        for (int i = 0; i < 512; i += 32) {
            DirEntry* entry = (DirEntry*)(sector + i);
            if (entry->Name[0] == 0x00) return; // fin de entradas
            if ((entry->Attr & 0x0F) == 0x0F) continue; // LFN
            if (entry->Name[0] == 0xE5) continue; // archivo borrado

            // Copiar nombre y ext, limpiando espacios al final
            char name[13] = {0};
            // Nombre base (8 bytes)
            int n = 0;
            for (int j = 0; j < 8 && entry->Name[j] != ' '; ++j)
                name[n++] = entry->Name[j];
            // Extensión (3 bytes)
            if (entry->Name[8] != ' ') {
                name[n++] = '.';
                for (int j = 8; j < 11 && entry->Name[j] != ' '; ++j)
                    name[n++] = entry->Name[j];
            }
            name[n] = '\0';

            Console::write("- ");
            Console::println(name);
        }
    }
}
