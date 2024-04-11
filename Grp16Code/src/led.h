#include <stdint.h>

#ifndef __team16led__
#define __team16led__

/// @brief Initialize hardware used to control LEDs
void led_init();

/// @brief Shift bits
/// @param data 
void led_shiftOut(uint16_t data);

/// @brief Test code for LEDs
void led_test();

#endif