/* ============================================================================
 * FAT16 Filesystem Driver — BlackHole OS
 *
 * Minimal FAT16 implementation for reading, writing, listing, and deleting
 * files in the root directory of our 1 MB ATA disk image.
 * ========================================================================= */

#include "fat16.h"
#include "../drivers/ata.h"
#include "../include/stdio.h"
#include "../include/string.h"
#include "../kernel/memory.h"

/* Cached layout values computed from the BPB */
static uint32_t fat_start_lba;       /* First sector of FAT1 */
static uint32_t root_dir_start_lba;  /* First sector of root directory */
static uint32_t data_start_lba;      /* First sector of data region */
static uint32_t root_dir_sectors;    /* Number of sectors the root dir occupies */
static uint16_t bytes_per_sector;
static uint8_t  sectors_per_cluster;
static uint16_t root_entry_count;
static uint16_t fat_size_sectors;
static uint8_t  num_fats;

/* ---- Helpers ---- */

/* Convert a cluster number to its starting LBA */
static uint32_t cluster_to_lba(uint16_t cluster) {
    return data_start_lba + ((uint32_t)(cluster - 2) * sectors_per_cluster);
}

/* Read a single FAT16 entry (2 bytes) for a given cluster */
static uint16_t fat_read_entry(uint16_t cluster) {
    uint32_t fat_offset = (uint32_t)cluster * 2;
    uint32_t fat_sector = fat_start_lba + (fat_offset / 512);
    uint32_t entry_offset = fat_offset % 512;

    uint8_t buf[512];
    ata_read_sector(fat_sector, buf);
    return *(uint16_t *)&buf[entry_offset];
}

/* Write a single FAT16 entry for a given cluster (writes to both FAT copies) */
static void fat_write_entry(uint16_t cluster, uint16_t value) {
    uint32_t fat_offset = (uint32_t)cluster * 2;
    uint32_t fat_sector_off = fat_offset / 512;
    uint32_t entry_offset = fat_offset % 512;

    for (uint8_t f = 0; f < num_fats; f++) {
        uint32_t sector = fat_start_lba + (f * fat_size_sectors) + fat_sector_off;
        uint8_t buf[512];
        ata_read_sector(sector, buf);
        *(uint16_t *)&buf[entry_offset] = value;
        ata_write_sector(sector, buf);
    }
}

/* Find a free cluster in the FAT. Returns cluster number or 0 on failure. */
static uint16_t fat_alloc_cluster(void) {
    /* Scan clusters 2 through a reasonable max (our 1 MB disk has ~2000 sectors / 1 sec per cluster = ~2000 clusters) */
    for (uint16_t c = 2; c < 2048; c++) {
        if (fat_read_entry(c) == 0x0000) {
            return c;
        }
    }
    return 0; /* Disk full */
}

/* Convert user-facing filename to FAT16 8.3 uppercase padded name.
 * "hello.txt" -> "HELLO   TXT" */
static void to_fat_name(const char *input, char *fat_name) {
    memset(fat_name, ' ', 11);
    int i = 0, j = 0;

    /* Copy the base name (up to 8 chars, or until '.') */
    while (input[i] && input[i] != '.' && j < 8) {
        char c = input[i];
        if (c >= 'a' && c <= 'z') c -= 32; /* uppercase */
        fat_name[j++] = c;
        i++;
    }

    /* Skip to extension */
    if (input[i] == '.') {
        i++;
        j = 8;
        while (input[i] && j < 11) {
            char c = input[i];
            if (c >= 'a' && c <= 'z') c -= 32;
            fat_name[j++] = c;
            i++;
        }
    }
}

/* Compare an 11-byte FAT name with the user input */
static int fat_name_match(const char *input, const uint8_t *dir_name) {
    char fat_name[11];
    to_fat_name(input, fat_name);
    for (int i = 0; i < 11; i++) {
        if (fat_name[i] != (char)dir_name[i]) return 0;
    }
    return 1;
}

/* ---- Public API ---- */

int fat16_init(void) {
    uint8_t sector[512];
    ata_read_sector(0, sector);

    bpb_t *bpb = (bpb_t *)sector;

    /* Sanity check */
    if (bpb->bytes_per_sector != 512) {
        printf("[FAT16] ERROR: Unsupported sector size %u\n", bpb->bytes_per_sector);
        return -1;
    }

    bytes_per_sector    = bpb->bytes_per_sector;
    sectors_per_cluster = bpb->sectors_per_cluster;
    root_entry_count    = bpb->root_entry_count;
    fat_size_sectors    = bpb->fat_size_16;
    num_fats            = bpb->num_fats;

    fat_start_lba       = bpb->reserved_sectors;
    root_dir_start_lba  = fat_start_lba + ((uint32_t)num_fats * fat_size_sectors);
    root_dir_sectors    = ((uint32_t)root_entry_count * 32 + 511) / 512;
    data_start_lba      = root_dir_start_lba + root_dir_sectors;

    printf("> FAT16 initialized. Data starts at LBA %u.\n", data_start_lba);
    return 0;
}

void fat16_list_root(void) {
    uint8_t sector[512];
    int file_count = 0;

    for (uint32_t s = 0; s < root_dir_sectors; s++) {
        ata_read_sector(root_dir_start_lba + s, sector);
        fat16_dir_entry_t *entries = (fat16_dir_entry_t *)sector;
        int entries_per_sector = 512 / sizeof(fat16_dir_entry_t);

        for (int i = 0; i < entries_per_sector; i++) {
            if (entries[i].name[0] == 0x00) goto done; /* No more entries */
            if (entries[i].name[0] == 0xE5) continue;   /* Deleted */
            if (entries[i].attr & FAT16_ATTR_VOLUME_ID) continue;
            if (entries[i].attr & FAT16_ATTR_LONG_NAME) continue;

            /* Print 8.3 filename */
            char name[13];
            int n = 0;
            for (int j = 0; j < 8; j++) {
                if (entries[i].name[j] != ' ')
                    name[n++] = entries[i].name[j];
            }
            if (entries[i].name[8] != ' ') {
                name[n++] = '.';
                for (int j = 8; j < 11; j++) {
                    if (entries[i].name[j] != ' ')
                        name[n++] = entries[i].name[j];
                }
            }
            name[n] = '\0';

            printf("  %-13s %u bytes\n", name, entries[i].file_size);
            file_count++;
        }
    }
done:
    if (file_count == 0) {
        printf("  (empty directory)\n");
    }
}

int fat16_read_file(const char *name, uint8_t *buffer, uint32_t buf_size) {
    uint8_t sector[512];

    for (uint32_t s = 0; s < root_dir_sectors; s++) {
        ata_read_sector(root_dir_start_lba + s, sector);
        fat16_dir_entry_t *entries = (fat16_dir_entry_t *)sector;
        int entries_per_sector = 512 / sizeof(fat16_dir_entry_t);

        for (int i = 0; i < entries_per_sector; i++) {
            if (entries[i].name[0] == 0x00) return -1;
            if (entries[i].name[0] == 0xE5) continue;
            if (!fat_name_match(name, entries[i].name)) continue;

            /* Found the file! Walk the cluster chain. */
            uint16_t cluster = entries[i].first_cluster_lo;
            uint32_t file_size = entries[i].file_size;
            uint32_t bytes_read = 0;
            uint32_t cluster_bytes = (uint32_t)sectors_per_cluster * 512;

            while (cluster >= 2 && cluster < 0xFFF8 && bytes_read < file_size) {
                uint32_t lba = cluster_to_lba(cluster);
                for (uint8_t sec = 0; sec < sectors_per_cluster && bytes_read < file_size; sec++) {
                    uint8_t tmp[512];
                    ata_read_sector(lba + sec, tmp);
                    uint32_t to_copy = file_size - bytes_read;
                    if (to_copy > 512) to_copy = 512;
                    if (bytes_read + to_copy > buf_size) to_copy = buf_size - bytes_read;
                    memcpy(buffer + bytes_read, tmp, to_copy);
                    bytes_read += to_copy;
                }
                cluster = fat_read_entry(cluster);
            }
            return (int)bytes_read;
        }
    }
    return -1; /* File not found */
}

int fat16_write_file(const char *name, const uint8_t *data, uint32_t size) {
    /* First, delete existing file with same name (overwrite semantics) */
    fat16_delete_file(name);

    /* Allocate clusters for the data */
    uint32_t cluster_bytes = (uint32_t)sectors_per_cluster * 512;
    uint32_t clusters_needed = (size + cluster_bytes - 1) / cluster_bytes;
    if (clusters_needed == 0) clusters_needed = 1;

    uint16_t first_cluster = 0;
    uint16_t prev_cluster = 0;

    for (uint32_t c = 0; c < clusters_needed; c++) {
        uint16_t cluster = fat_alloc_cluster();
        if (cluster == 0) {
            printf("[FAT16] ERROR: Disk full!\n");
            return -1;
        }

        /* Mark as end-of-chain for now */
        fat_write_entry(cluster, 0xFFFF);

        /* Link previous cluster to this one */
        if (prev_cluster != 0) {
            fat_write_entry(prev_cluster, cluster);
        }

        if (c == 0) first_cluster = cluster;
        prev_cluster = cluster;

        /* Write data to this cluster's sectors */
        uint32_t lba = cluster_to_lba(cluster);
        uint32_t offset = c * cluster_bytes;
        for (uint8_t sec = 0; sec < sectors_per_cluster; sec++) {
            uint8_t buf[512];
            memset(buf, 0, 512);
            uint32_t data_offset = offset + (uint32_t)sec * 512;
            if (data_offset < size) {
                uint32_t to_copy = size - data_offset;
                if (to_copy > 512) to_copy = 512;
                memcpy(buf, data + data_offset, to_copy);
            }
            ata_write_sector(lba + sec, buf);
        }
    }

    /* Find a free directory entry and write it */
    uint8_t sector[512];
    for (uint32_t s = 0; s < root_dir_sectors; s++) {
        ata_read_sector(root_dir_start_lba + s, sector);
        fat16_dir_entry_t *entries = (fat16_dir_entry_t *)sector;
        int entries_per_sector = 512 / sizeof(fat16_dir_entry_t);

        for (int i = 0; i < entries_per_sector; i++) {
            if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
                /* Free slot! Populate it. */
                memset(&entries[i], 0, sizeof(fat16_dir_entry_t));
                to_fat_name(name, (char *)entries[i].name);
                entries[i].attr = FAT16_ATTR_ARCHIVE;
                entries[i].first_cluster_lo = first_cluster;
                entries[i].file_size = size;

                ata_write_sector(root_dir_start_lba + s, sector);
                return 0;
            }
        }
    }

    printf("[FAT16] ERROR: Root directory full!\n");
    return -1;
}

int fat16_delete_file(const char *name) {
    uint8_t sector[512];

    for (uint32_t s = 0; s < root_dir_sectors; s++) {
        ata_read_sector(root_dir_start_lba + s, sector);
        fat16_dir_entry_t *entries = (fat16_dir_entry_t *)sector;
        int entries_per_sector = 512 / sizeof(fat16_dir_entry_t);

        for (int i = 0; i < entries_per_sector; i++) {
            if (entries[i].name[0] == 0x00) return -1;
            if (entries[i].name[0] == 0xE5) continue;
            if (!fat_name_match(name, entries[i].name)) continue;

            /* Free the cluster chain first */
            uint16_t cluster = entries[i].first_cluster_lo;
            while (cluster >= 2 && cluster < 0xFFF8) {
                uint16_t next = fat_read_entry(cluster);
                fat_write_entry(cluster, 0x0000);
                cluster = next;
            }

            /* Mark directory entry as deleted */
            entries[i].name[0] = 0xE5;
            ata_write_sector(root_dir_start_lba + s, sector);
            return 0;
        }
    }
    return -1; /* Not found */
}
