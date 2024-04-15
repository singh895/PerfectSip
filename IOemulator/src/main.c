#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "io.h"
#include "ttl.h"
#include "bluart.h"
#include "bluetooth.h"

void print_usage();
void* io_thread(void* args);
void* parse_thread(void* args);

FILE* ttl_file = NULL;


int main(int argc, char** argv)
{
	// Get file pointer for ttl from arguments
	if (argc == 1)
	{
		ttl_file = fopen("debug.txt", "w");
		if (!ttl_file) {
			dprintf(STDOUT_FILENO, "FILE ERROR: debug.txt could not be opened\n");
			print_usage();
			return 1;
		}
	}
	else if (argc == 2)
	{
		ttl_file = fopen(argv[1], "w");
		if (!ttl_file) {
			dprintf(STDOUT_FILENO, "FILE ERROR: %s could not be opened\n", argv[1]);
			print_usage();
			return 1;
		}		
	}
	else 
	{
		print_usage();
		return 1;
	}

	// Start IO read thread
	pthread_t io_tid, parse_tid;

	// Set IO nonblocking
	fcntl (0, F_SETFL, O_NONBLOCK);

	pthread_create(&io_tid, NULL, io_thread, NULL);
	pthread_create(&parse_tid, NULL, parse_thread, NULL);

	dprintf(STDOUT_FILENO, "Thread created, ready to recieve input: \n");

	dev_prints(&ttl_device, "TTL device emulation is set up\n");

	dprintf(STDOUT_FILENO, "bluetooth_init() starting...\n");
	dev_prints(&ttl_device, "bluetooth_init() starting...\n");

	bluetooth_init();

	dprintf(STDOUT_FILENO, "bluetooth_init() completed!\n");
	dev_prints(&ttl_device, "bluetooth_init() completed!\n");

	sleep(10);

	// Close file
	fclose(ttl_file);
}

void* io_thread(void* args) 
{
	char gchar; 
	ssize_t size;
	io_error_t ioerror;

	while(1) {

		// Read one character
		size = read(STDIN_FILENO, &gchar, 1);
		if (size != 1) {
			continue;
		}

		// Store character on buffer
		ioerror = buff_putchar(&bluart_device.input_buffer, gchar);
		if (ioerror) {
			dprintf(STDOUT_FILENO, "IO ERROR for bluart input buffer. Check debug file for info. \n");
			dev_print_ioerror(&ttl_device, ioerror);
			return NULL;
		}
	}
	
}

void* parse_thread(void* args)
{
	int counter = 0;

	while(1) {

		bluetooth_updatestate();

		counter++;

		if (counter % 128 == 0) {
			dev_prints(&ttl_device, "Heartbeat ");
			dev_print_int(&ttl_device, counter/128);
			dev_prints(&ttl_device, "\n");
		}

		usleep(1000);
	}
}

void print_usage()
{
	dprintf(STDOUT_FILENO, "Usage: ./IOemulator <debug filepath>\n");
	dprintf(STDOUT_FILENO, "If executed with no arguments, debug.txt will be used for debug info. \n");
	dprintf(STDOUT_FILENO, "For one argument, the arguement will be used as a file path as described above.\n");
}