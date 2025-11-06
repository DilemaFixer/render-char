#ifndef CMAP
#define CMAP

#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "source.h"
#include "ttf.h"

#define CMAP_TAG "cmap"

#pragma pack(1)

typedef struct cmap_header {
    uint16_t version;
    uint16_t numTables;
} cmap_header;

typedef struct cmap_encoding_record {
    uint16_t platformID;
    uint16_t encodingID;
    uint32_t subtableOffset;
} cmap_encoding_record;

#pragma pack()

uint16_t get_glyph_index(ttf_source *source, uint32_t unicode);
uint16_t get_glyph_index_format4(uint8_t *table, uint32_t unicode);

#endif
