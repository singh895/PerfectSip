#include "io.h"

#ifndef __team16bt__
#define __team16bt__

typedef enum bluetooth_state {
    BT_IDLE,        // Waiting for data
    BT_DATA,        // Data send is not a response from module

    BT_OK_1,        // Recieved "O" character from module
    BT_OK_2,        // Recieved "OK" from module
    BT_OK_3,        // Recieved "OK+" from module

    BT_LOST_01,     // Recieved "OK+L" from module
    BT_LOST_02,     // Recieved "OK+LO" from module
    BT_LOST_03,     // Recieved "OK+LOS" from module
    BT_LOST_04,     // Recieved "OK+LOST" from module
    BT_LOST_05,     // Recieved "OK+LOST:" from module

    BT_CONN_01,     // Recieved "OK+C" from module
    BT_CONN_02,     // Recieved "OK+CO" from module
    BT_CONN_03,     // Recieved "OK+CON" from module
    BT_CONN_04,     // Recieved "OK+CONN" from module
    BT_CONN_05,     // Recieved "OK+CONN:" from module

    BT_S,           // Recieved "OK+S"

    BT_START_02,    // Recieved "OK+ST"
    BT_START_03,    // Recieved "OK+STA"
    BT_START_04,    // Recieved "OK+STAR"
    BT_START_05,    // Recieved "OK+START"

    BT_Set_02,      // Recieved "OK+Se" from module
    BT_Set_03,      // Recieved "OK+Set" from module
    BT_Set_04,      // Recieved "OK+Set:" from module

    // Used by CHAR, NOTI, NOTP, 
} bluetooth_state_t;

typedef enum bluetooth_prevcmd {
    BTCMD_NONE, // No command sent yet
    BTCMD_AT,
    BTCMD_CHAR, // Expect "Set" response
    BTCMD_NOTI, // Expect "Set" response
    BTCMD_NOTP, // Expect "Set" response
    BTCMD_NAME, // Expect "Set" response
    BTCMD_UUID, // Expect "Set" response
    BTCMD_START
    
} bluetooth_prevcmd_t;

struct bluetooth_info
{
    char waiting;    // 0 if command can be sent, 1 if waiting for a command's response
    char started;    // 0 if not started, 1 if started
    char connected;  // 0 if not connected, 1 if connected

    char chari[5];   // Characteristic
    char uuid[5];    // UUID
    char name[14];   // Name of device
    char mac[13];    // Mac address of connected device

    char noti;       // 0 for not notify, 1 for notify (when connect or disconnect)
    char notp;       // 0 to not send connected mac address, 1 to send mac address info with noti 

    char cmd_waiting;

    int data_len;                   // Tracks length of data stored in OK response
    bluetooth_state_t state;     // Current state
    bluetooth_prevcmd_t prevcmd; // Previously executed command
};


extern struct IODevice bluetooth_device;
extern struct bluetooth_info bt_info;

/// @brief Initializes bluetooth
void bluetooth_init();

/// @brief Sets AT command
/// @return Error code
io_error_t bluetooth_AT();

/// @brief Sets characteristic
/// @param char_str Characteristic value
/// @return Error code
io_error_t bluetooth_CHAR(char* char_str);

/// @brief Sets notify information
/// @param notify notify info
/// @return Error code
io_error_t bluetooth_NOTI(char notify);

/// @brief Sets notify mode
/// @param notifyp notify mode
/// @return Error code
io_error_t bluetooth_NOTP(char notifyp);

/// @brief Sets device name
/// @param name_str name
/// @return Error code
io_error_t bluetooth_NAME(char* name_str);

/// @brief Sets UUID
/// @param uuid_str uid
/// @return Error code
io_error_t bluetooth_UUID(char* uuid_str);

/// @brief Sends start command
/// @return Error code
io_error_t bluetooth_START();

/// @brief Sends a character to bluetooth hardware
/// @param c Character to send
/// @return Error code
io_error_t bluetooth_putchar(char c);

/// @brief Gets a character from bluetooth
/// @param c Recieve character pointer 
/// @return Error code
io_error_t bluetooth_getchar(char *c);

#endif