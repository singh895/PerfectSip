#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "io.h"
#include "bluart.h"
#include "ttl.h"

char bluart_inputbuf[128];
// char bluart_outputbuf[64];

struct IODevice bluart_device = {
    .poption = PRINT_DEFAULT,
    .foption = FLUSH_NOBUFF,
	.device_getchar = bluart_getchar,
    .device_putchar = bluart_putchar,
	.input_buffer = {
		.size = 128,
		.buffer = &bluart_inputbuf[0],


		.sidx = 0,
		.eidx = 0,
		.length = 0
	},
	.output_buffer = {
		.size = 64,
		// .buffer = &bluart_outputbuf[0],
        .buffer = 0,

		.sidx = 0,
		.eidx = 0,
		.length = 0
	}
};

extern struct IODevice ttl_device;

void bluart_init() 
{
    dev_prints(&ttl_device, "Called bluart_init()\n");
    return;
}

void reset_bluetooth() {
    dev_prints(&ttl_device, "Called reset_bluetooth()\n");
    return;
}

io_error_t bluart_putchar(char c) {

	ssize_t size;

	size = write(STDOUT_FILENO, &c, 1);

	if (size != 1) {
		return IO_DEVERROR0;
	}

    return IO_SUCCESS;
}

io_error_t bluart_getchar(char* c) {

    return IO_NOT_IMPLMENT;
}