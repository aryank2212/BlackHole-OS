#ifndef ATA_H
#define ATA_H

#include <stdint.h>

/* Disk Sector Size is standard 512 bytes */
#define ATA_SECTOR_SIZE 512

/* Initialize the Primary ATA Bus */
void ata_init(void);

/* Wait for the disk controller to be ready */
void ata_wait_bsy(void);
void ata_wait_drq(void);

/* Read a 512-byte sector via Logical Block Addressing (LBA) */
void ata_read_sector(uint32_t lba, uint8_t *buffer);

/* Write a 512-byte sector via Logical Block Addressing (LBA) */
void ata_write_sector(uint32_t lba, uint8_t *buffer);

#endif /* ATA_H */
