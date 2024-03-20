#ifndef __team16io__
#define __team16io__

typedef enum io_error {
    IO_SUCCESS      = 0,    // Operation completed successfully
    IO_NULL_BUFFOBJ = 1,    // Buffer object is null
    IO_NULL_BUFFER  = 2,    // Buffer pointer is null
    IO_NO_BUFFER    = 3,    // Buffer doesn't have a size
    IO_BUFFER_FULL  = 4,    // Buffer is full
    IO_BUFFER_EMPTY = 5,    // Buffer is empty
    IO_ARG_ERROR    = 6,    // Error with given arguments
    IO_DEVERROR0    = 8,    // Device specific error 0
    IO_DEVERROR1    = 9,    // Device specific error 1
    IO_DEVERROR2    = 10,   // Device specific error 2
    IO_DEVERROR3    = 11,   // Device specific error 3
    IO_NOT_IMPLMENT = 17    // Not implemented   
} io_error_t;

typedef enum print_option {
    PRINT_DEFAULT   = 0,    // Print string without terminating character 
    PRINT_NULLCHAR  = 1     // Print string with terminating character
} print_option_t;

typedef enum flush_option {
    FLUSH_NOBUFF    = 0,    // Print directly to device without using buffer
    FLUSH_NOFLUSH   = 1,    // Print to buffer, flushing only when needed
    FLUSH_FLUSH     = 2,    // Flush when needed and when end of string is reached
} flush_option_t;

/// @brief Struct containing information about an IO buffer
struct IOBuffer
{
    int size;       // Size of buffer, not the size of contents in the buffer
    char* buffer;   // Pointer to buffer

    int sidx;       // Start position of buffer
    int eidx;       // End position of buffer
    int length;     // Gets size of contents in the buffer
};

/// @brief Struct containing information about an IODevice
struct IODevice
{
    print_option_t poption;
    flush_option_t foption;

    io_error_t (*device_getchar)(char* c);
    io_error_t (*device_putchar)(char c);
    // int (*dflush)(struct IODevice* device); // Device driver to output data on buffer to the io device. 

    struct IOBuffer input_buffer;   // Info about input buffer
    struct IOBuffer output_buffer;  // Info about output buffer
};

// --- Buffer functions --- //

/// @brief Puts a character onto a buffer
/// @param buffer Buffer object pointer
/// @param ch Character to be placed on buffer
/// @return Error code
io_error_t buff_putchar(struct IOBuffer* buffer, char c);

/// @brief Gets a character from buffer
/// @param buffer Buffer object pointer 
/// @param ch Pointer to output character
/// @return Error code
io_error_t buff_getchar(struct IOBuffer* buffer, char* c);

/// @brief Prints a string onto a buffer
/// @param buffer Buffer object pointer
/// @param str String to be printed
/// @return Error code
io_error_t buff_prints(struct IOBuffer* buffer, char* str);

/// @brief Copies string from buffer to string pointer. 
/// @param str String pointer 
/// @param n Size of string pointer
/// @param buffer Buffer object pointer
/// @return Error code
io_error_t buff_gets(char* str, int n, struct IOBuffer* buffer);


// --- Device functions --- //

/// @brief Prints string directly to device
/// @param device Device struct pointer 
/// @param str String pointer 
/// @return Error code
io_error_t dev_printsd(struct IODevice* device, char* str);

/// @brief Prints string to device
/// @param device Device struct pointer
/// @param str String pointer
/// @param option Describes how flushing should be handled =
/// @return Error code
io_error_t dev_prints(struct IODevice* device, char* str);

/// @brief Sends all contents of the buffer to the device
/// @param device Device struct pointer
/// @return Error code
io_error_t dev_flush(struct IODevice* device);


#endif