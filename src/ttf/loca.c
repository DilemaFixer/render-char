#include "loca.h"


void load_as_short(ttf_source *source, head_table* head, uint16_t index, uint32_t* glyph_offset, uint32_t* glyph_length) {
    ttf_table_record loca_record = {0};
    try_load_table_record(source, &loca_record, LOCA_TAG);
    
    uint16_t *table = (uint16_t*)((uint8_t*)source->data + ntohl(loca_record.offset));
    uint16_t start = ntohs(table[index]);
    uint16_t end = ntohs(table[index + 1]);
    
    *glyph_offset = start * 2;
    *glyph_length = (end - start) * 2;
}

void load_as_long(ttf_source *source, head_table* head, uint16_t index, uint32_t* glyph_offset, uint32_t* glyph_length) {
    ttf_table_record loca_record = {0};
    try_load_table_record(source, &loca_record, LOCA_TAG);
    
    uint32_t *table = (uint32_t*)((uint8_t*)source->data + ntohl(loca_record.offset));
    uint32_t start = ntohl(table[index]);
    uint32_t end = ntohl(table[index + 1]);
    
    *glyph_offset = start;
    *glyph_length = end - start;
}
