#ifndef SOURCE
#define SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#pragma pack(1)


typedef struct ttf_source {
    void *data;
    size_t size;
} ttf_source;

#pragma pack()

int load_ttf_source(ttf_source *source, const char *filename);

#endif
