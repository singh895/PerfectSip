

#include "bluetooth.h"
#include "bluart.h"

char bluetooth_inputbuf[128];
char bluetooth_outputbuf[64];

struct IODevice bluetooth_device = {
    .poption = PRINT_DEFAULT,
    .foption = FLUSH_NOBUFF,
	.device_getchar = bluetooth_getchar,
    .device_putchar = bluetooth_putchar,
	.input_buffer = {
		.size = 128,
		// .buffer = &bluart_inputbuf[0],
        .buffer = 0,


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

void bluetooth_init()
{
    bluart_init();
}

io_error_t bluetooth_putchar(char c)
{
	return IO_NOT_IMPLMENT;
}

io_error_t bluetooth_getchar(char *c)
{
	return IO_NOT_IMPLMENT;
}