#include "glyph.h"


glyph_t* try_load_glyph(ttf_source *source, uint32_t glyph_offset, uint32_t glyph_length) {
    
    ttf_table_record glyf_record = {0};
    try_load_table_record(source, &glyf_record, FLYPH_TAG);

    uint8_t *glyf_table = (uint8_t*)source->data + ntohl(glyf_record.offset);
    uint8_t *glyph_data = glyf_table + glyph_offset;

    glyph_header *gh = (glyph_header*)glyph_data;
    int16_t numberOfContours = (int16_t)ntohs(gh->numberOfContours);


    if (numberOfContours <= 0) {
        elog("glyph has zero contours");
    }

    uint16_t *endPtsOfContours = (uint16_t*)(glyph_data + sizeof(glyph_header));
    uint16_t numPoints = ntohs(endPtsOfContours[numberOfContours - 1]) + 1;
    
    uint8_t *ptr = (uint8_t*)(endPtsOfContours + numberOfContours);
    uint16_t instructionLength = ntohs(*(uint16_t*)ptr);
    ptr += 2 + instructionLength;
    

    glyph_t *glyph = (glyph_t*)malloc(sizeof(glyph_t));
    glyph->count = numPoints;

    // Read flags
    glyph->flags = malloc(numPoints);
    int flag_idx = 0;
    while (flag_idx < numPoints) {
        uint8_t flag = *ptr++;
        glyph->flags[flag_idx++] = flag;
        
        if (flag & 0x08) {  // REPEAT
            uint8_t repeat_count = *ptr++;
            for (int i = 0; i < repeat_count; i++) {
                glyph->flags[flag_idx++] = flag;
            }
        }
    }
    
    // Read X coordinates
    glyph->x_poss = malloc(numPoints * sizeof(int16_t));
    int16_t x = 0;
    for (int i = 0; i < numPoints; i++) {
        uint8_t flag = glyph->flags[i];
        
        if (flag & 0x02) {  // X_SHORT_VECTOR
            uint8_t val = *ptr++;
            x += (flag & 0x10) ? val : -val;
        } else if (!(flag & 0x10)) {
            int16_t delta = (int16_t)ntohs(*(uint16_t*)ptr);
            x += delta;
            ptr += 2;
        }
        
        glyph->x_poss[i] = x;
    }
    
    // Read Y coordinates
    glyph->y_poss = malloc(numPoints * sizeof(int16_t));
    int16_t y = 0;
    for (int i = 0; i < numPoints; i++) {
        uint8_t flag = glyph->flags[i];
        
        if (flag & 0x04) {  // Y_SHORT_VECTOR
            uint8_t val = *ptr++;
            y += (flag & 0x20) ? val : -val;
        } else if (!(flag & 0x20)) {
            int16_t delta = (int16_t)ntohs(*(uint16_t*)ptr);
            y += delta;
            ptr += 2;
        }
        
        glyph->y_poss[i] = y;
    }


    glyph->xMin = (int16_t)ntohs(gh->xMin);
    glyph->yMin = (int16_t)ntohs(gh->yMin);
    glyph->xMax = (int16_t)ntohs(gh->xMax);
    glyph->yMax = (int16_t)ntohs(gh->yMax);
    glyph->endPtsOfContours = endPtsOfContours;
    glyph->numberOfContours = numberOfContours;

    return glyph;
}
