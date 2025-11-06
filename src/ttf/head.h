#ifndef HEAD
#define HEAD

#include <stdint.h>
#include <stdbool.h>

#include "source.h"
#include "ttf.h"

#define HEAD_TAG "head"

#pragma pack(1)

typedef uint32_t fixed_t;

typedef struct head_table {
    uint16_t majorVersion;
    uint16_t minorVersion;
    fixed_t fontRevision;
    uint32_t checksumAdjustment;
    uint32_t magicNumber;
    uint16_t flags;
    uint16_t unitsPerEm;
    int64_t created;
    int64_t modified;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    uint16_t macStyle;
    uint16_t lowestRecPPEM;
    int16_t fontDirectionHint;
    int16_t indexToLocFormat;
    int16_t glyphDataFormat;
} head_table;

#pragma pack()

head_table* try_load_head_table(ttf_source *source);
bool is_short_format(head_table *head);
#endif
