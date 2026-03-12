#ifndef FAT16_H
#define FAT16_H

#include "../include/stdint.h"

/* ---- BIOS Parameter Block (BPB) — first 62 bytes of sector 0 ---- */
typedef struct {
    uint8_t  jmp[3];              /* Jump instruction (EB xx 90) */
    uint8_t  oem_name[8];        /* OEM name */
    uint16_t bytes_per_sector;   /* Almost always 512 */
    uint8_t  sectors_per_cluster;/* Cluster size in sectors */
    uint16_t reserved_sectors;   /* Sectors before the first FAT (includes BPB) */
    uint8_t  num_fats;           /* Number of FAT copies (usually 2) */
    uint16_t root_entry_count;   /* Max root directory entries (FAT16) */
    uint16_t total_sectors_16;   /* Total sectors (if fits in 16 bits) */
    uint8_t  media_type;         /* Media descriptor byte */
    uint16_t fat_size_16;        /* Sectors per FAT */
    uint16_t sectors_per_track;  /* Geometry */
    uint16_t num_heads;          /* Geometry */
    uint32_t hidden_sectors;     /* Sectors before partition */
    uint32_t total_sectors_32;   /* Total sectors (if > 65535) */
} __attribute__((packed)) bpb_t;

/* ---- FAT16 Directory Entry (32 bytes) ---- */
typedef struct {
    uint8_t  name[11];           /* 8.3 filename (space-padded) */
    uint8_t  attr;               /* File attributes */
    uint8_t  reserved;           /* Reserved for WinNT */
    uint8_t  crt_time_tenth;     /* Creation time (tenths of sec) */
    uint16_t crt_time;           /* Creation time */
    uint16_t crt_date;           /* Creation date */
    uint16_t last_acc_date;      /* Last access date */
    uint16_t first_cluster_hi;   /* High 16 bits of first cluster (0 for FAT16) */
    uint16_t wrt_time;           /* Last write time */
    uint16_t wrt_date;           /* Last write date */
    uint16_t first_cluster_lo;   /* Low 16 bits of first cluster */
    uint32_t file_size;          /* File size in bytes */
} __attribute__((packed)) fat16_dir_entry_t;

/* File attribute flags */
#define FAT16_ATTR_READ_ONLY  0x01
#define FAT16_ATTR_HIDDEN     0x02
#define FAT16_ATTR_SYSTEM     0x04
#define FAT16_ATTR_VOLUME_ID  0x08
#define FAT16_ATTR_DIRECTORY  0x10
#define FAT16_ATTR_ARCHIVE    0x20
#define FAT16_ATTR_LONG_NAME  0x0F

/* Initialize the FAT16 driver by reading the BPB */
int fat16_init(void);

/* List all files in the root directory */
void fat16_list_root(void);

/* Read a file by name into buffer. Returns bytes read, or -1 on error. */
int fat16_read_file(const char *name, uint8_t *buffer, uint32_t buf_size);

/* Write data to a file (create or overwrite). Returns 0 on success. */
int fat16_write_file(const char *name, const uint8_t *data, uint32_t size);

/* Delete a file by name. Returns 0 on success. */
int fat16_delete_file(const char *name);

#endif /* FAT16_H */
