#include <stdio.h>

#include "ttl.h"

extern FILE* ttl_file;

struct IODevice ttl_device = {
	.poption = PRINT_DEFAULT,
	.foption = FLUSH_NOBUFF,
	.device_getchar = ttl_getchar,
	.device_putchar = ttl_putchar,
	.input_buffer = {	// No input buffer unless someone wants to get debug input
		.size = 0,
		.buffer = 0,

		.sidx = 0,
		.eidx = 0,
		.length = 0
	},
	.output_buffer = {	// No output buffer because printing directly to ttl device
		.size = 0,
		.buffer = 0,

		.sidx = 0,
		.eidx = 0,
		.length = 0
	}
};

// Function copied from James DeGruccio's 362 Lab11
void ttl_init() {
    return;
}

io_error_t ttl_putchar(char c) {

	int pchar;
	
	pchar = fputc(c, ttl_file);

	if ((char)pchar != c) {
		return IO_DEVERROR0;
	}

	return IO_SUCCESS;
}

io_error_t ttl_getchar(char *c) {
	// Not needed for emulator
	return IO_NOT_IMPLMENT;
}