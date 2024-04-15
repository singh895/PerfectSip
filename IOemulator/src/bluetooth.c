

#include "bluetooth.h"
#include "bluart.h"
#include "ttl.h"


char bluetooth_inputbuf[128];
char bluetooth_outputbuf[64];

struct IODevice bluetooth_device = {
    .poption = PRINT_DEFAULT,
    .foption = FLUSH_NOBUFF,
	.device_getchar = bluetooth_getchar,
    .device_putchar = bluetooth_putchar,
	.input_buffer = {
		.size = 128,
		.buffer = &bluetooth_inputbuf[0],
        .buffer = 0,


		.sidx = 0,
		.eidx = 0,
		.length = 0
	},
	.output_buffer = {
		.size = 0,
		// .buffer = &bluart_outputbuf[0],
        .buffer = 0,

		.sidx = 0,
		.eidx = 0,
		.length = 0
	}
};

struct bluetooth_info bt_info = {
	.waiting = 0,
	.started = 0,
	.connected = 0,

	.chari = "FFE1",
	.uuid = "FFE0",
	.name = "DSD TECH",
	.mac = "000000000000",

	.noti = 0,
	.notp = 0,

	.cmd_waiting = 0,

	.data_len = 0,
	.state = BT_IDLE,
	.prevcmd = BTCMD_NONE
};

void bluetooth_init()
{
	io_error_t ioerror;

    bluart_init();
	reset_bluetooth();

	// Make it so OK+CONN and OK+LOST is sent on
	ioerror = bluetooth_NOTI(1);
	if (ioerror) dev_print_ioerror(&ttl_device, ioerror);

	// Make it so MAC address is sent on connections/disconnections
	bluetooth_NOTP(1);
	if (ioerror) dev_print_ioerror(&ttl_device, ioerror);

	// Set characteristic and service uuid
	bluetooth_CHAR("FFE1");
	if (ioerror) dev_print_ioerror(&ttl_device, ioerror);
	bluetooth_UUID("FFE0");
	if (ioerror) dev_print_ioerror(&ttl_device, ioerror);

	// Set name
	bluetooth_NAME("PerfectSip");
	if (ioerror) dev_print_ioerror(&ttl_device, ioerror);

	// Start bluetooth module
	bluetooth_START();
	if (ioerror) dev_print_ioerror(&ttl_device, ioerror);
}

io_error_t bluetooth_AT() 
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_AT;
	ioerror = dev_prints(&bluart_device, "AT");
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

io_error_t bluetooth_CHAR(char* char_str)
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_CHAR;
	ioerror = dev_prints(&bluart_device, "AT+CHAR0x");
	if (ioerror) return ioerror;
	ioerror = dev_prints(&bluart_device, char_str);
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

io_error_t bluetooth_NOTI(char notify)
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_NOTI;
	if (notify) {
		ioerror = dev_prints(&bluart_device, "AT+NOTI1");
	}
	else {
		ioerror = dev_prints(&bluart_device, "AT+NOTI0");
	}
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

io_error_t bluetooth_NOTP(char notifyp)
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_NOTP;
	if (notifyp) {
		ioerror = dev_prints(&bluart_device, "AT+NOTP1");
	}
	else {
		ioerror = dev_prints(&bluart_device, "AT+NOTP0");
	}
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

io_error_t bluetooth_NAME(char* name_str)
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_NAME;
	ioerror = dev_prints(&bluart_device, "AT+NAME");
	if (ioerror) return ioerror;
	ioerror = dev_prints(&bluart_device, name_str);
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

io_error_t bluetooth_UUID(char* uuid_str)
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_UUID;
	ioerror = dev_prints(&bluart_device, "AT+UUID0x");
	if (ioerror) return ioerror;
	ioerror = dev_prints(&bluart_device, uuid_str);
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

io_error_t bluetooth_START()
{
	io_error_t ioerror;

	// Wait for prev command response if needed
	while (bt_info.cmd_waiting);

	// Send command
	bt_info.prevcmd = BTCMD_START;
	ioerror = dev_prints(&bluart_device, "AT+START");
	if (ioerror) return ioerror;

	bt_info.cmd_waiting = 1;

	return IO_SUCCESS;
}

/// @brief Update the state machine using available data
void bluetooth_updatestate()
{
	io_error_t ioerr;

	// No data to parse, do nothing and return
	if (bluart_device.input_buffer.length == 0) {
		return;
	}

	// Loop until no data to parse
	while (bluart_device.input_buffer.length != 0) {

		char newchar;
		
		// Get char from uart buffer
		ioerr = buff_getchar(&bluart_device.input_buffer, &newchar);
		if (ioerr) {
			dev_print_ioerror(&ttl_device, ioerr);
		}

		// NOTE: If going to the idle state, cmd_waiting should be cleared
		switch (bt_info.state) {
		case BT_IDLE:
			switch (newchar) {
			case 'O':	// Could be start of OK+ from module
				bt_info.state = BT_OK_1;
				break;
			case '\n':	// Stay in idle if newline or creturn
			case '\r':
				bt_info.state = BT_IDLE;
				break;
			default:	// Not a command, save data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_DATA:
			switch (newchar) {
			case '\n':	// Go to idle, end of data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_OK_1:
			switch (newchar) {
			case 'K':	// Could be start of OK+ from module
				bt_info.state = BT_OK_2;
				break;
			case '\n':	// Wasnt start of OK+, save sent "O"
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Wasnt start of OK+, save sent "O"
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_OK_2:
			switch (newchar) {
			case '+':	// Start of OK+ from module
				bt_info.state = BT_OK_3;
				break;
			case '\n':	// OK response from module
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				break;
			default: 	// Wasnt start of OK+, save sent "OK"
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_OK_3:
			switch (newchar) {
			case 'L':
				bt_info.state = BT_LOST_01;
				break;
			case 'C':
				bt_info.state = BT_CONN_01;
				break;

			case 'S':
				bt_info.state = BT_S;
				break;

			case '\n':	// Unexpected return, treat OK+ like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;

			default: 	// Unexpected value, treat OK+ like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_LOST_01:
			switch (newchar) {
			case 'O':
				bt_info.state = BT_LOST_02;
				break;
			case '\n':	// Unexpected return, treat OK+L like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Unexpected data, treat OK+L like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_LOST_02:
			switch (newchar) {
			case 'S':
				bt_info.state = BT_LOST_03;
				break;
			case '\n':	// Unexpected return, treat OK+LO like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "LO");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Unexpected data, treat OK+LO like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "LO");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_LOST_03:
			switch (newchar) {
			case 'T':
				bt_info.state = BT_LOST_04;
				break;
			case '\n':	// Unexpected return, treat OK+LOS like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "LOS");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Unexpected data, treat OK+LOS like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "LOS");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_LOST_04:
			switch (newchar) {
			case ':':
				bt_info.state = BT_LOST_05;
				break;
			case '\n':	// Recieved OK+LOST, device disconnected
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.connected = 0;
				break;
			default: 	// Unexpected data, treat OK+LOST like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "LOST");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_LOST_05:
			switch (newchar) {
			case '\n':	// Recieved OK+LOST, device disconnected
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.connected = 0;
				break;
			default:	// This should be the mac address of disconnected device. Dont handle data
				break;
			}
			break;
		case BT_CONN_01:
			switch (newchar) {
			case 'O':
				bt_info.state = BT_CONN_02;
				break;
			case '\n':	// Unexpected return, treat OK+C like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Unexpected data, treat OK+C like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_CONN_02:
			switch (newchar) {
			case 'N':
				bt_info.state = BT_CONN_03;
				break;
			case '\n':	// Unexpected return, treat OK+CO like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "CO");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Unexpected data, treat OK+CO like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "CO");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_CONN_03:
			switch (newchar) {
			case 'N':
				bt_info.state = BT_CONN_04;
				break;
			case '\n':	// Unexpected return, treat OK+CON like data
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "CON");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 	// Unexpected data, treat OK+CON like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "CON");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_CONN_04:
			switch (newchar) {
			case ':':
				bt_info.state = BT_CONN_05;
				break;
			case '\n':	// Recieved OK+CONN, device connected
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.connected = 0;
				break;
			default: 	// Unexpected data, treat OK+CONN like data
				bt_info.state = BT_DATA; 
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "CONN");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_CONN_05:
			switch (newchar) {
			case '\n':	// Recieved OK+CONN, device disconnected
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.connected = 0;
				bt_info.data_len = 0;
				break;
			default:	// This should be the mac address of disconnected device. Dont handle data
				// If length of data is within size of mac address array, store data in array
				if (bt_info.data_len < 12) {
					bt_info.mac[bt_info.data_len] = newchar;
				}
				bt_info.data_len++;
				break;
			}
			break;
		case BT_S:
			switch (newchar) {
			case 'T':
				bt_info.state = BT_START_02;
				break;
			case 'e':
				bt_info.state = BT_Set_02;
				break;
			case '\n':	// Unexpected return, treat OK+S like data
			case '\r': 
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default:	// Unexpected data, treat OK+S like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_START_02:
			switch (newchar) {
			case 'A':
				bt_info.state = BT_START_03;
				break;
			case '\n':
			case '\r':	// Unexpected data, treat OK+ST like data
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "ST");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default:	// Unexpected data, treat OK+ST like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "ST");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_START_03:
			switch (newchar) {
			case 'R':
				bt_info.state = BT_START_04;
				break;
			case '\n':
			case '\r':	// Unexpected data, treat OK+STA like data
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "STA");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default:	// Unexpected data, treat OK+STA like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "STA");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_START_04:
			switch (newchar) {
			case 'T':
				bt_info.state = BT_START_05;
				break;
			case '\n':
			case '\r':	// Unexpected data, treat OK+STAR like data
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "STAR");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default:	// Unexpected data, treat OK+STAR like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "STAR");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_START_05:
			switch (newchar) {
			case '\n':
			case '\r':	// Recieved OK+START, module started
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.started = 1;
				break;
			default:
				break;
			} 
			break;
		case BT_Set_02:
			switch (newchar) {
			case 't':
				bt_info.state = BT_Set_03;
				break;
			case '\n':
			case '\r':	// Unexpected data, treat OK+Se like data
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "Se");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default:	// Unexpected data, treat OK+Se like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "Se");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_Set_03:
			switch (newchar) {
			case ':':
				bt_info.state = BT_Set_04;
				break;
			case 'N':
				bt_info.state = BT_SetName_04;
				break;
			case '\n':
			case '\r':	// Unexpected data, treat OK+Set like data
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "Set");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default:	// Unexpected data, treat OK+Set like data
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "Set");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_Set_04:

			switch (newchar) {
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.data_len = 0;
				break;
			
			default:
				switch (bt_info.prevcmd) {
				case BTCMD_CHAR:
					// Ignores '0x' characters
					if (2 <= bt_info.data_len && bt_info.data_len < 6) {
						bt_info.chari[bt_info.data_len-2] = newchar;
					}
					break;
				case BTCMD_NOTI:
					if (newchar == '0') {
						bt_info.noti = 0;
					}
					else if (newchar == '1') {
						bt_info.noti = 1;
					}
					break;
				case BTCMD_NOTP:
					if (newchar == '0') {
						bt_info.notp = 0;
					}
					else if (newchar == '1') {
						bt_info.notp = 1;
					}
					break;
				case BTCMD_NAME:
					if (bt_info.data_len < 13) {
						bt_info.name[bt_info.data_len] = newchar;
					}
					break;
				case BTCMD_UUID:
					// Ignores '0x' characters
					if (2 <= bt_info.data_len && bt_info.data_len < 6) {
						bt_info.uuid[bt_info.data_len-2] = newchar;
					}
					break;
				default:
					break;
				}

				bt_info.data_len++;
				break;
			}
			
			break;
		
		case BT_SetName_04: 
			switch (newchar) {
			case 'a':
				bt_info.state = BT_SetName_05;
				break;
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetN");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetN");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_SetName_05: 
			switch (newchar) {
			case 'm':
				bt_info.state = BT_SetName_06;
				break;
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetNa");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetNa");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_SetName_06: 
			switch (newchar) {
			case 'e':
				bt_info.state = BT_SetName_07;
				break;
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetNam");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetNam");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_SetName_07: 
			switch (newchar) {
			case ':':
				bt_info.state = BT_SetName_08;
				break;
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetName");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			default: 
				bt_info.state = BT_DATA;
				buff_prints(&bluetooth_device.input_buffer, "OK+");
				buff_prints(&bluetooth_device.input_buffer, "SetName");
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_SetName_08: 
			switch (newchar) {
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.cmd_waiting = 0;
				bt_info.data_len = 0;
				break;
			default: 
				if (bt_info.data_len < 13) {
					bt_info.name[bt_info.data_len] = newchar;
				}
				bt_info.data_len++;
				break;
			}
			
			break;

		default:
			bt_info.state = BT_IDLE;
			bt_info.cmd_waiting = 0;
			break;
		}
	}
}

io_error_t bluetooth_putchar(char c)
{
	if (bt_info.cmd_waiting) {
		return IO_DEVERROR0;
	}
	else if (!bt_info.connected) {
		return IO_DEVERROR1;
	}

	return bluart_putchar(c);
}

io_error_t bluetooth_getchar(char *c)
{
	bluetooth_updatestate();

	return IO_SUCCESS;
}