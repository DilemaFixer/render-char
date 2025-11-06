#ifndef TTF_H
#define TTF_H

#include <fcntl.h>
#include <unistd.h>  
#include <sys/mman.h> 
#include <sys/stat.h>  
#include <arpa/inet.h>  
#include <stdio.h>       
#include <stdint.h>      
#include "source.h"

#pragma pack(1)

typedef struct ttf_header {
    uint32_t sfntVersion;
    uint16_t numTables;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
} ttf_header;

typedef struct ttf_table_record {
    char tag[4];
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
} ttf_table_record;

#pragma pack()

int find_table_record(ttf_source *source, ttf_table_record *record, const char* name);
void try_load_table_record(ttf_source *source, ttf_table_record *record, const char name[4]);

#endif
