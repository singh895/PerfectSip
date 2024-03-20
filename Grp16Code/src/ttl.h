#ifndef __team16ttl__
#define __team16ttl__

#include "io.h"

extern struct IODevice ttl_device;

void ttl_init();

io_error_t ttl_putchar(char c);
io_error_t ttl_getchar(char *c);

#endif
