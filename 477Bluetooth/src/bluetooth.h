#include "io.h"

#ifndef __team16bt__
#define __team16bt__

extern struct IODevice bluetooth_device;

void bluetooth_init();

io_error_t bluetooth_putchar(char c);
io_error_t bluetooth_getchar(char *c);

#endif