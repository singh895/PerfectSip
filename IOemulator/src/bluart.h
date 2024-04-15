#ifndef __team16blueart__
#define __team16blueart__

#include "io.h"

extern struct IODevice bluart_device;

/// @brief Initializes USART1 used with bluetooth
void bluart_init();

/// @brief Resets the bluetooth module using EN pin
void reset_bluetooth();

/// @brief Sends a character to USART hardware
/// @param c Character to send
/// @return Error code
io_error_t bluart_putchar(char c);

/// @brief Gets a character from USART hardware
/// @param c Recieve character pointer 
/// @return Error code
io_error_t bluart_getchar(char* c);

#endif