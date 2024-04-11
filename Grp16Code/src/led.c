#include "stm32f0xx.h"
#include <stdint.h>

#include "led.h"
#include "timing.h"

void led_test() {
    while (1) {
        for (int i = 0; i < 16; i++) {
            led_shiftOut(1 << i); // Shift a single '1' through the LEDs
            nano_wait(10000000);    
        }

        for (int i = 16; i > 0; i--) {
            led_shiftOut(1 << i); // Shift a single '1' through the LEDs in the reverse direction
            nano_wait(10000000);    
        }
    }
}

void led_init() {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;                                                  // Enable GPIOC clock
    GPIOC->MODER |= GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0;    // Set PC3, PC4, and PC5 as outputs
}

void led_shiftOut(uint16_t data) {

    for (uint16_t i = 0; i < 16; i++) {
        GPIOC->BSRR = (data & 0x8000) ? (GPIO_BSRR_BS_3 | (1 << 2)) : (GPIO_BSRR_BR_3 | (1 << 2));
        GPIOC->BSRR = (1 << 4);
        GPIOC->BSRR = (1 << 4) << 16;
        data <<= 1;
    }
    GPIOC->BSRR = (1 << 5);
    GPIOC->BSRR = (1 << 5) << 16;
}