#include "source.h"

int load_ttf_source(ttf_source *source, const char *filename)
{
	struct stat info;
	int fd;
	source->data = MAP_FAILED;
	source->size   = 0;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		return -1;
	}
	if (fstat(fd, &info) < 0) {
		close(fd);
		return -1;
	}
	source->data = mmap(NULL, (size_t) info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	source->size   = (uint_fast32_t) info.st_size;
	close(fd);
	return source->data == MAP_FAILED ? -1 : 0;
}

