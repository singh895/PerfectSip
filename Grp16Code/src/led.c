#include "stm32f0xx.h"
#include <stdint.h>

#include "led.h"
#include "timing.h"

void led_test() {
    while (1) {

        GPIOB->BSRR |= GPIO_BSRR_BS_0;
        for (int i = 0; i < 16; i++) {
            led_shiftOut(1 << i); // Shift a single '1' through the LEDs
            nano_wait(10000000);    
        }

        GPIOB->BSRR |= GPIO_BSRR_BR_0;
        for (int i = 16; i > 0; i--) {
            led_shiftOut(1 << i); // Shift a single '1' through the LEDs in the reverse direction
            nano_wait(10000000);    
        }
    }
}

void led_init() {
    // OLD
    // PC3: Data            -> PB13
    // PC4: Clock           -> PB14
    // PC5: Set register    -> PB15

    // Enable GPIOB clock
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;          

    // Set PB0, PB13, PB14, and PB15 as outputs
    GPIOB->MODER |= GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0 | GPIO_MODER_MODER0_0;   
}

void led_shiftOut(uint16_t data) {

    for (uint16_t i = 0; i < 16; i++) {
        // Output high or low depending on most significant bit
        GPIOB->BSRR = (data & 0x8000) ? (GPIO_BSRR_BS_13 | (1 << 2)) : (GPIO_BSRR_BR_13 | (1 << 2));
        nano_wait(30);

        // Shift in value into register
        GPIOB->BSRR = GPIO_BSRR_BS_14;
        nano_wait(10);

        GPIOB->BSRR = GPIO_BSRR_BR_14;
        nano_wait(10);

        // Shift data to ready next bit
        data <<= 1;
    }

    // Set register
    GPIOB->BSRR = GPIO_BSRR_BS_15;
    GPIOB->BSRR = GPIO_BSRR_BR_15;
}