#ifndef LOCA
#define LOCA

#include <stdint.h>
#include "source.h"

#include "head.h"
#include "ttf.h"

#define LOCA_TAG "loca"

void load_as_short(ttf_source *source, head_table* head, uint16_t index, uint32_t* glyph_offset, uint32_t* glyph_length);
void load_as_long(ttf_source *source, head_table* head, uint16_t index, uint32_t* glyph_offset, uint32_t* glyph_length);

#endif
