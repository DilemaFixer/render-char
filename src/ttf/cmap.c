#include "cmap.h"

uint16_t get_glyph_index(ttf_source *source, uint32_t unicode) {
    ttf_table_record cmap_record = {0};
    try_load_table_record(source, &cmap_record, CMAP_TAG);

    cmap_header *cmap = (cmap_header*)((char*)source->data + htonl(cmap_record.offset));
    uint16_t numTables = ntohs(cmap->numTables);
    cmap_encoding_record *record = (cmap_encoding_record*)((char*)cmap + sizeof(cmap_header));

    uint32_t cmap_offset = ntohl(cmap_record.offset);
    uint32_t record_offset = ntohl(record->subtableOffset);
    uint8_t *subtable = source->data + cmap_offset + record_offset;

    return get_glyph_index_format4(subtable, unicode);
}

uint16_t get_glyph_index_format4(uint8_t *table, uint32_t unicode)
{
    uint8_t *p = table;
    uint16_t segCountX2 = ntohs(*(uint16_t *)(p + 6));
    uint16_t segCount = segCountX2 / 2;

    uint16_t *endCode       = (uint16_t *)(p + 14);
    uint16_t *startCode     = endCode + segCount + 1;
    int16_t  *idDelta       = (int16_t *)(startCode + segCount);
    uint16_t *idRangeOffset = (uint16_t *)(idDelta + segCount);

    for (int i = 0; i < segCount; i++) {
        uint16_t start = ntohs(startCode[i]);
        uint16_t end   = ntohs(endCode[i]);
        int16_t delta  = ntohs(idDelta[i]);
        uint16_t range = ntohs(idRangeOffset[i]);

        if (unicode >= start && unicode <= end) {
            if (range == 0) {
                return (unicode + delta) & 0xFFFF;
            } else {
                uint16_t *glyphIndexAddr =
                    (uint16_t *)((uint8_t *)&idRangeOffset[i] + range + 2 * (unicode - start));
                uint16_t glyph = ntohs(*glyphIndexAddr);
                if (glyph != 0)
                    glyph = (glyph + delta) & 0xFFFF;
                return glyph;
            }
        }
    }
    return 0;
}
