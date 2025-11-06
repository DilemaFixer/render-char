#ifndef GLYPH
#define GLYPH

#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "source.h"
#include "ttf.h"
#include "logger.h"

#define FLYPH_TAG "glyf"

#pragma pack(1)

typedef struct glyph_header {
    int16_t numberOfContours;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
} glyph_header;

#pragma pack()

typedef struct glyph_t {
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    uint8_t *flags;
    int16_t *x_poss;
    int16_t *y_poss;
    uint16_t count;
    int16_t numberOfContours;
    uint16_t *endPtsOfContours;
} glyph_t;

glyph_t* try_load_glyph(ttf_source *source, uint32_t glyph_offset, uint32_t glyph_length);

#endif
