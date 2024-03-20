#include "stm32f0xx.h"
#include "ttl.h"

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
    //Configure pins
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;
    GPIOC->MODER |= GPIO_MODER_MODER12_1;              //Pin C12
    GPIOD->MODER |= GPIO_MODER_MODER2_1;              //Pin D2
    GPIOC->AFR[1] &= ~0x000F0000;           //Set AF to USART5 for C12
    GPIOC->AFR[1] |=  0x00020000;
    GPIOD->AFR[0] &= ~0x00000F00;           //Set AF to USART5 for D2
    GPIOD->AFR[0] |=  0x00000200;
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;   //Enable clock to USART5

    //Configure USART5
    USART5->CR1 &= ~USART_CR1_UE;                       //Disable USART
    USART5->CR1 &= ~(USART_CR1_M | USART_CR1_M << 16);  //Word size of 8 bits
    //USART5->CR1 |= USART_CR1_M;
    USART5->CR1 &= ~USART_CR1_OVER8;                    //16x oversampling
    USART5->CR1 &= ~USART_CR1_PCE;                      //No parity
    USART5->CR2 &= ~USART_CR2_STOP;                     //One stop bit
    USART5->BRR = 0x1A1;                                //Baud rate of 115200
    USART5->CR1 |= USART_CR1_TE | USART_CR1_RE;         //Setting TE and RE bits to enable transiver and reciever
    USART5->CR1 |= USART_CR1_UE;    //Enable USART

    //Wait for TE and RE bits to be acknowledge by checking TEACK and REACK
    while (!(USART5->ISR & USART_ISR_REACK) || !(USART5->ISR & USART_ISR_TEACK));    //If REACK or TEACH are 0, wait for both to be 1

	// Set up interrupt for USART5
	USART5->CR1 |= USART_CR1_RXNEIE;  	//Generate interrupt when ORE=1 or RXNE=1
    NVIC->ISER[0] |= 1<<USART3_8_IRQn; 	//Enable interrupt in ISER
	
}

io_error_t ttl_putchar(char c) {
	if (c == '\n' || c == '\r') {
		while(!(USART5->ISR & USART_ISR_TXE));	// Wait for USART to be ready
		USART5->TDR = '\r';
		while(!(USART5->ISR & USART_ISR_TXE));	// Wait for USART to be ready
		USART5->TDR = '\n';
	}
	else {
		while(!(USART5->ISR & USART_ISR_TXE));	// Wait for USART to be ready
		USART5->TDR = c;
	}	

	return IO_SUCCESS;
}

io_error_t ttl_getchar(char *c) {
	while (!(USART5->ISR & USART_ISR_RXNE));
	*c = USART5->RDR;

	return IO_SUCCESS;
}

void USART3_8_IRQHandler(void) {
	char ch;

	ch = USART5->RDR;

    ttl_putchar(ch);
}