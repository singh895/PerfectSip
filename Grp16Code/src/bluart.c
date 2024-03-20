#include <stdio.h>

#include "stm32f0xx.h"
#include "timing.h"
#include "io.h"
#include "bluart.h"
#include "ttl.h"

// char bluart_inputbuf[128];
// char bluart_outputbuf[64];

struct IODevice bluart_device = {
    .poption = PRINT_DEFAULT,
    .foption = FLUSH_NOBUFF,
	.device_getchar = bluart_getchar,
    .device_putchar = bluart_putchar,
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

extern struct IODevice ttl_device;

void bluart_init() 
{
    // Setup GPIO pins
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;                          // Enable RCC clock to GPIO A
    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1; // Select alt function mode
    GPIOA->MODER |= GPIO_MODER_MODER8_0;                        // Set pin 8 as output mode

    // Select alternate function to AF1 for pins 9 and 10
    GPIOA->AFR[1] &= ~0x00000FF0;   // Make sure AFR for A9 and A10 is set to AF0 first 
    GPIOA->AFR[1] |=  0x00000110;   // Set AF1 for A9 and A10  
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   // Enable Clock to USART1

    // Configure USART1
    USART1->CR1 &= ~USART_CR1_UE;   // Disable USART while configuring
    USART1->CR1 &= ~(USART_CR1_M | USART_CR1_M << 16);  // Word size of 8 bits
    USART1->CR1 &= ~USART_CR1_OVER8;    //16x oversampling
    USART1->CR1 &= ~USART_CR1_PCE;      //No parity
    USART1->CR2 &= ~USART_CR2_STOP;     //One stop bit
    USART1->BRR =  (48000000 / 9600);   // Set baud rate of 9600, default HM-19 baud rate
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE; //Setting TE and RE bits to enable transiver and reciever
    USART1->CR1 |= USART_CR1_UE;    //Enable USART

    //Wait for TE and RE bits to be acknowledge by checking TEACK and REACK
    while (!(USART1->ISR & USART_ISR_REACK) || !(USART1->ISR & USART_ISR_TEACK));    //If REACK or TEACH are 0, wait for both to be 1

    // Enable USART1 interrupt
    USART1->CR1 |= USART_CR1_RXNEIE;    // Generate interrupt when RXNE
    NVIC->ISER[0] |= 1<<USART1_IRQn;    // Enable interrupt in ISER
                                        // Hope this is enough for interrupts

    // Reset bluetooth module 
    reset_bluetooth();
}

void reset_bluetooth() {
    GPIOA->BSRR |= GPIO_BSRR_BR_8;      // Set (active high) reset to low 
    nano_wait(1010000000);               // Wait for 1010ms
    GPIOA->BSRR |= GPIO_BSRR_BS_8;      // Set reset back to high   
}

io_error_t bluart_putchar(char c) {
	while(!(USART1->ISR & USART_ISR_TXE));	// Wait for USART to be ready
	USART1->TDR = c;                        // Send character

    return IO_SUCCESS;
}

io_error_t send_float_over_bluetooth(float fl) {
    // Convert temperature to string
    char temp_str[10]; // Adjust the size based on your temperature range
    sprintf(temp_str, "%.2f", fl);

    // Send each character of the string over Bluetooth
    for (int i = 0; temp_str[i] != '\0'; i++) {
        bluart_putchar(temp_str[i]);
    }

    return IO_SUCCESS;
}

io_error_t bluart_getchar(char* c) {
    // Check if recieve data register is not empty
    while (!(USART1->ISR & USART_ISR_RXNE));
    *c = USART1->RDR;   // Get data from recieve data register

    return IO_SUCCESS;
}

void USART1_IRQHandler(void) {

    char ch;

    ch = USART1->RDR;

    ttl_putchar(ch);

    buff_putchar(&bluart_device.input_buffer, ch);  // Store read char in buffer

    // ttl_device.device_putchar(ch);  // Send char to ttl for debugging purposes
}