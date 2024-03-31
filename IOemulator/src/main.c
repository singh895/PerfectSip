#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "io.h"
#include "ttl.h"
#include "bluart.h"

void print_usage();
void* io_thread(void* args);

FILE* ttl_file = NULL;


int main(int argc, char** argv)
{
	// Get file pointer from arguments
	if (argc == 1)
	{
		ttl_file = fopen("debug.txt", "w");

		if (!ttl_file) {
			fprintf(stdout, "FILE ERROR: debug.txt could not be opened\n");
			print_usage();
			return 1;
		}
	}
	else if (argc == 2)
	{
		ttl_file = fopen(argv[1], "w");

		if (!ttl_file) {
			fprintf(stdout, "FILE ERROR: %s could not be opened\n", argv[1]);
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
	pthread_t thread_id;

	pthread_create(&thread_id, NULL, io_thread, NULL);

	fprintf(stdout, "Thread created, ready to recieve input: \n");

	dev_prints(&ttl_device, "TTL device emulation is set up\n");

	sleep(10);

	// Close file
	fclose(ttl_file);
}

void* io_thread(void* args) 
{
	int gchar; 
	io_error_t ioerror;

	while(1) {

		// Get character
		gchar = fgetc(stdin);
		if (gchar == EOF) {
			fprintf(stdout, "Error in io_thread. Got EOF from fgetc()\n");
			return NULL;
		}

		// Store character on buffer
		ioerror = buff_putchar(&bluart_device.input_buffer, gchar);
		if (ioerror) {
			fprintf(stdout, "IO ERROR for bluart input buffer. Check debug file for info. \n");
			dev_print_ioerror(&ttl_device, ioerror);
			return NULL;
		}
	}
}

void print_usage()
{
	fprintf(stdout, "Usage: ./IOemulator <debug filepath>\n");
	fprintf(stdout, "If executed with no arguments, debug.txt will be used for debug info. \n");
	fprintf(stdout, "For one argument, the arguement will be used as a file path as described above.\n");
}