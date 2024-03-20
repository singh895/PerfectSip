#include "stm32f0xx.h"
#include "timing.h"
#include "io.h"
#include "ttl.h"
#include "bluart.h"


void internal_clock();


int main(void)
{
	// Set clock to 48Mhz
	internal_clock();

	// Init ttl device 
	ttl_init();

	dev_prints(&ttl_device, "Hello World!\n");
	dev_prints(&ttl_device, "Using dprints on STM32F0\n");
	dev_prints(&ttl_device, "Im so excited to get Bluetooth working!!!\n\n");

	dev_prints(&ttl_device, "Init bluart...\n");

	bluart_init();

	dev_prints(&ttl_device, "Init finished....\n");

	// char tmpch;



	dev_prints(&bluart_device, "AT");

	// bluart_getchar(&tmpch);
	// bluart_getchar(&tmpch);

	nano_wait(1000000);

	dev_prints(&bluart_device, "AT+NAMEPerfectSip");

	dev_prints(&ttl_device, "After sending AT\n");
	
	while (1) {
		bluart_putchar('1');
		bluart_putchar('6');
		bluart_putchar('4');

		nano_wait(1000000000);	// Wait one second
	}
};
