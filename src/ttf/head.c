#include "head.h"

head_table* try_load_head_table(ttf_source *source) {
    
    ttf_table_record head_record = {0};
    try_load_table_record(source, &head_record, HEAD_TAG);

    return (head_table *)((uint8_t*)source->data + htonl(head_record.offset));
}

bool is_short_format(head_table *head) {
    int16_t indexToLocFormat = (int16_t)ntohs(head->indexToLocFormat);
    return indexToLocFormat == 0;
}
