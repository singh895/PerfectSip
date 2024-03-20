#include "io.h"

io_error_t buff_putchar(struct IOBuffer* buffer, char c)
{
    // Check for null pointer
    if (!buffer) {
        return IO_NULL_BUFFOBJ;
    }

    // Check for null buffer pointer
    if (!buffer->buffer) {
        return IO_NULL_BUFFER;
    }

    // Check for buffer of size 0
    if (buffer->size == 0) {
        return IO_NO_BUFFER;
    }

    // Check for full buffer
    if (buffer->sidx == buffer->eidx && buffer->length == buffer->size) {
        return IO_BUFFER_FULL;
    }

    // Check if eidx is at the end of the memory block
    if (buffer->eidx == buffer->size-1) {
        buffer->buffer[buffer->size-1] = c;    // Put char on buffer
        buffer->eidx = 0;                       // Wrap end idx to start of block
        buffer->length++;                       // Update buffer length
    }
    else {
        buffer->buffer[buffer->eidx] = c;  // Put char on buffer
        buffer->eidx++;                     // Update end idx for next char to put on buffer
        buffer->length++;                   // Update buffer length
    }

    return IO_SUCCESS;
}

io_error_t buff_getchar(struct IOBuffer* buffer, char* c)
{
    int ch; 

    // Check for null pointer
    if (!buffer) {
        return IO_NULL_BUFFOBJ;
    }

    // Check for null buffer pointer
    if (!buffer->buffer) {
        return IO_NULL_BUFFER;
    }

    // Check for null character pointer
    if (!c) {
        return IO_ARG_ERROR;
    }

    // Check for buffer of size 0
    if (buffer->size == 0) {
        *c = '\0';
        return IO_NO_BUFFER;
    }

    // Check for empty buffer
    if (buffer->sidx == buffer->eidx && buffer->length == 0) {
        *c = '\0';
        return IO_BUFFER_EMPTY;
    }

    // Check if sidx is at the end of the memory block
    if (buffer->sidx == buffer->size-1) {
        ch = buffer->buffer[0]; // Get char from start of buffer
        buffer->sidx = 0;       // Wrap start idx to start of block
        buffer->length--;       // Update buffer length
    }
    else {
        ch = buffer->buffer[buffer->sidx];  // Get char from start idx
        buffer->sidx++;                     // Remove char from buffer
        buffer->length--;                   // Update buffer length
    }

    *c = ch;

    return IO_SUCCESS;
}

io_error_t buff_prints(struct IOBuffer* buffer, char* str)
{
    int i = 0;  // Var for iterating through string
    io_error_t putc_error;

    // Check for null pointer
    if (!buffer) {
        return IO_NULL_BUFFOBJ;
    }

    // Check for null string pointer 
    if (!str) {
        return IO_ARG_ERROR;
    }

    // Print each character in buffer to string
    while (str[i] != '\0') {
        putc_error = buff_putchar(buffer, str[i]);

        if (putc_error) {
            return putc_error;
        }

        i++;
    }

    return IO_SUCCESS;
}

io_error_t buff_gets(char* str, int n, struct IOBuffer* buffer)
{
    int i = 0;
    io_error_t getc_error;

    // Check for null pointer 
    if (!buffer) {
        return IO_NULL_BUFFOBJ;
    }

    // Check for null string pointer 
    if (!str) {
        return IO_ARG_ERROR;
    }

    // String should have size greater that zero  
    if (n <= 0) {
        return IO_ARG_ERROR;
    }

    // Loop until string is full
    while (i < n - 1) {
        getc_error = buff_getchar(buffer, &str[i]);

        // Return success value if buffer is depleted
        if (getc_error == IO_BUFFER_EMPTY) {
            return IO_SUCCESS;
        }
        // Otherwise return error
        else if (getc_error) {
            return getc_error;
        }
    }

    // Make sure terminating character is at end of string
    str[n-1] = '\0';

    // State that string buffer is full
    return IO_BUFFER_FULL;
}

io_error_t dev_printsd(struct IODevice* device, char* str) 
{

    int i = 0;  // Var for iterating through string
    io_error_t putc_error;

    // Check for null pointer
    if (!device) {
        return IO_NULL_BUFFOBJ;
    }

    // Check for null string pointer 
    if (!str) {
        return IO_ARG_ERROR;
    }

    // Print each character in buffer to string
    while (str[i] != '\0') {
        putc_error = device->device_putchar(str[i]);

        if (putc_error) {
            return putc_error;
        }

        i++;
    }

    if (device->poption == PRINT_NULLCHAR) {
        device->device_putchar('\0');
    }

    return IO_SUCCESS;
}

io_error_t dev_prints(struct IODevice* device, char* str) {

    int i = 0;  // Var for iterating through string
    io_error_t putc_error;

    // Check for null pointer
    if (!device) {
        return IO_NULL_BUFFOBJ;
    }

    // Check for null string pointer 
    if (!str) {
        return IO_ARG_ERROR;
    }

    // Just call dev_printsd for this option
    if (device->foption == FLUSH_NOBUFF) {
        return dev_printsd(device, str);
    }

    // Loop through string
    while (str[i] != '\0') {
        
        putc_error = buff_putchar(&device->output_buffer, str[i]);

        // If buffer is full, flush buffer and try again
        if (putc_error == IO_BUFFER_FULL) {

            putc_error = dev_flush(device);
            if (putc_error) {
                return putc_error;
            }
            
            putc_error = buff_putchar(&device->output_buffer, str[i]);
        }

        // Return error if error occured
        if (putc_error) {
            return putc_error;
        }

        i++;
    }

    if (device->poption == PRINT_NULLCHAR) {
        buff_putchar(&device->output_buffer, '\0');
    }

    // Flush if option says so
    if (device->foption == FLUSH_FLUSH) {
        dev_flush(device);
        if (putc_error) {
            return putc_error;
        }
    }

    return IO_SUCCESS;
}

io_error_t dev_flush(struct IODevice* device) {

    char c;
    io_error_t io_error;

    // Check for null pointer 
    if (!device) {
        return IO_NULL_BUFFOBJ;
    }

    // Loop until output buffer is empty
    while (device->output_buffer.length > 0) {
        // Get char from buffer
        io_error = buff_getchar(&device->output_buffer, &c);
        if (io_error) {
            return io_error;
        }

        // Output char to device
        io_error = device->device_putchar(c);
        if (io_error) {
            return io_error;
        }
    }

    return IO_SUCCESS;
}