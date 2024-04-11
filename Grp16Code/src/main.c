#include "stm32f0xx.h"
#include <stdio.h> 
#include "timing.h"
#include "io.h"
#include "ttl.h"
#include "bluart.h"
#include "temperature.h"
#include "led.h"


void internal_clock();


int main(void)
{
	// Set clock to 48Mhz
	internal_clock();

	setup_serial();
    setup_adc();
    setup_gpio();
    setup_button_interrupt();

	// Init ttl device 
	ttl_init();

	dev_prints(&ttl_device, "Hello World!\n");
	dev_prints(&ttl_device, "Using dprints on STM32F0\n");

	dev_prints(&ttl_device, "Init led...\n");
	led_init();

	dev_prints(&ttl_device, "led init'd, running test code...\n");

	led_test();

	// dev_prints(&ttl_device, "Init bluart...\n");

	// bluart_init();

	// dev_prints(&ttl_device, "Init finished....\n");

	// dev_prints(&bluart_device, "AT");

	// nano_wait(1000000);

	// dev_prints(&bluart_device, "AT+NAMEPerfectSip");

	// dev_prints(&ttl_device, "After sending AT\n");

	// while(1) {
    //     nano_wait(1000000000);
    //     float temperatureC = read_adc_voltage();
	// 	// float temperatureC = voltage_to_temp(voltage);
    //     // float temperatureF = celsius_to_fahrenheit(temperatureC);
    //     // print_voltage(voltage);
    //     // print_temp(voltage);
	// 	// char temp_str[32];

	// 	//convert temper
	// 	dev_prints(&bluart_device, "\nTemp val: ");
	// 	send_float_over_bluetooth(temperatureC);
    //     // check_temp_turnoff(voltage_to_temp(voltage));
    // }

	// while (1) {
	// 	bluart_putchar('1');
	// 	bluart_putchar('6');
	// 	bluart_putchar('4');

	// 	nano_wait(1000000000);	// Wait one second
	// }
}
