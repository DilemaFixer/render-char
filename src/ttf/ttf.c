#include <string.h>

#include "ttf.h"
#include "logger.h"


int find_table_record(ttf_source *source, ttf_table_record *record, const char name[4]) {
    (void) record;
    ttf_header *header = (ttf_header*)source->data;
    ttf_table_record *table_records = (ttf_table_record*)((char*)source->data + sizeof(ttf_header));

    for (uint16_t i = 0; i < header->numTables; i++) {
        ttf_table_record *rec = &table_records[i];
    
        if (memcmp(rec->tag, name, 4) == 0) {
            *record = *rec;
            return 0;
        }
    };

    return 1;
}


void try_load_table_record(ttf_source *source, ttf_table_record *record, const char name[4]) {
    if(find_table_record(source, record, name)) {
        elog("record about table '%s' not fount int ttf header", name);
    }
}
