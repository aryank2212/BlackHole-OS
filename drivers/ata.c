/* ============================================================================
 * primary ATA/IDE Hard Disk Driver — BlackHole OS
 *
 * Implements standard PIO Mode (Programmed I/O) blocking disk transfers 
 * for 28-bit Logical Block Addressing (LBA).
 * ========================================================================= */

#include "ata.h"
#include "../kernel/idt.h" /* For inb, outb, inw, outw */
#include "../include/stdio.h"
#include <stdint.h>

/* Primary IDE Controller I/O port bases */
#define ATA_PORT_DATA       0x1F0
#define ATA_PORT_ERROR      0x1F1
#define ATA_PORT_FEATURES   0x1F1
#define ATA_PORT_SECTOR_CNT 0x1F2
#define ATA_PORT_LBA_LO     0x1F3
#define ATA_PORT_LBA_MID    0x1F4
#define ATA_PORT_LBA_HI     0x1F5
#define ATA_PORT_DRIVE      0x1F6
#define ATA_PORT_COMMAND    0x1F7
#define ATA_PORT_STATUS     0x1F7

/* Commands */
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30
#define ATA_CMD_CACHE_FLUSH 0xE7

/* Status bits */
#define ATA_SR_ERR          0x01  /* Error */
#define ATA_SR_DRQ          0x08  /* Data Request Ready */
#define ATA_SR_BSY          0x80  /* Busy */

/* Wait until drive is no longer busy */
void ata_wait_bsy(void) {
    while(inb(ATA_PORT_STATUS) & ATA_SR_BSY);
}

/* Wait until drive requests data transfer */
void ata_wait_drq(void) {
    while(!(inb(ATA_PORT_STATUS) & ATA_SR_DRQ));
}

/* Internal helper to select the drive and format the LBA 28-bit address */
static void ata_pio_setup(uint32_t lba, uint8_t drive) {
    ata_wait_bsy();
    
    /* Select drive (0 = master, 1 = slave) + LBA top 4 bits */
    outb(ATA_PORT_DRIVE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_PORT_FEATURES, 0x00);
    outb(ATA_PORT_SECTOR_CNT, 1);
    outb(ATA_PORT_LBA_LO, (uint8_t)lba);
    outb(ATA_PORT_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PORT_LBA_HI, (uint8_t)(lba >> 16));
}

void ata_init(void) {
    /* Send IDENTIFY command to primary master to verify it exists */
    outb(ATA_PORT_DRIVE, 0xA0); 
    outb(ATA_PORT_SECTOR_CNT, 0);
    outb(ATA_PORT_LBA_LO, 0);
    outb(ATA_PORT_LBA_MID, 0);
    outb(ATA_PORT_LBA_HI, 0);
    outb(ATA_PORT_COMMAND, 0xEC); /* IDENTIFY command */
    
    uint8_t status = inb(ATA_PORT_STATUS);
    if (status == 0) {
        printf("[ATA] No primary IDE master drive found.\n");
        return;
    }
    
    ata_wait_bsy();
    /* Drain IDENTIFY data (256 words) we don't need right now */
    if (inb(ATA_PORT_STATUS) & ATA_SR_DRQ) {
        for (int i = 0; i < 256; i++) {
            inw(ATA_PORT_DATA);
        }
    }
    printf("> Primary ATA Drive initialized.\n");
}

void ata_read_sector(uint32_t lba, uint8_t *buffer) {
    ata_pio_setup(lba, 0); /* 0 = Primary Master */
    outb(ATA_PORT_COMMAND, ATA_CMD_READ_PIO);
    
    ata_wait_bsy();
    ata_wait_drq();
    
    /* Read 256 words (512 bytes) from the data port into our buffer */
    for (int i = 0; i < 256; i++) {
        uint16_t w = inw(ATA_PORT_DATA);
        buffer[i * 2] = (uint8_t)(w & 0xFF);
        buffer[i * 2 + 1] = (uint8_t)(w >> 8);
    }
}

void ata_write_sector(uint32_t lba, uint8_t *buffer) {
    ata_pio_setup(lba, 0);
    outb(ATA_PORT_COMMAND, ATA_CMD_WRITE_PIO);
    
    ata_wait_bsy();
    ata_wait_drq();
    
    /* Write 256 words to the data port from our buffer */
    for (int i = 0; i < 256; i++) {
        uint16_t w = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(ATA_PORT_DATA, w);
    }
    
    /* Tell drive to flush hardware cache */
    outb(ATA_PORT_COMMAND, ATA_CMD_CACHE_FLUSH);
    ata_wait_bsy();
}
