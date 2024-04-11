#include <stdio.h>

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
    //fprintf(stdout, "Called bluart_init()\n");
    return;
}

void reset_bluetooth() {
    //fprintf(stdout, "Called reset_bluetooth()\n");
    return;
}

io_error_t bluart_putchar(char c) {

    int pchar; 

    pchar = fputc(c, stdout);

    if ((char)pchar != c) {
		return IO_DEVERROR0;
	}

    return IO_SUCCESS;
}

io_error_t bluart_getchar(char* c) {



    return IO_NOT_IMPLMENT;
}