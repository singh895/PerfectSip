

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

	.data_len = 0,
	.state = BT_IDLE,
	.prevcmd = BTCMD_NONE
};

void bluetooth_init()
{
    bluart_init();
}

void bluetooth_updatestate()
{
	io_error_t ioerr;

	// No data to parse, do nothing and return
	if (bluetooth_device.input_buffer.length == 0) {
		return;
	}

	// Loop until no data to parse
	while (bluetooth_device.input_buffer.length != 0) {

		char newchar;
		
		// Get char from uart buffer
		ioerr = buff_getchar(&bluart_device.input_buffer, &newchar);
		if (ioerr) {
			dev_print_ioerror(&ttl_device, ioerr);
		}

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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				break;
			default: 	// Wasnt start of OK+, save sent "O"
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
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
				break;
			default: 	// Wasnt start of OK+, save sent "OK"
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;

			default: 	// Unexpected value, treat OK+ like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default: 	// Unexpected data, treat OK+L like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default: 	// Unexpected data, treat OK+LO like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default: 	// Unexpected data, treat OK+LOS like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
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
				bt_info.connected = 0;
				break;
			default: 	// Unexpected data, treat OK+LOST like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'L');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_LOST_05:
			switch (newchar) {
			case '\n':	// Recieved OK+LOST, device disconnected
			case '\r':
				bt_info.state = BT_IDLE;
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default: 	// Unexpected data, treat OK+C like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default: 	// Unexpected data, treat OK+CO like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'N');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default: 	// Unexpected data, treat OK+CON like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'N');
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
				bt_info.connected = 0;
				break;
			default: 	// Unexpected data, treat OK+CONN like data
				bt_info.state = BT_DATA; 
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'C');
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'N');
				buff_putchar(&bluetooth_device.input_buffer, 'N');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			}
			break;
		case BT_CONN_05:
			switch (newchar) {
			case '\n':	// Recieved OK+CONN, device disconnected
			case '\r':
				bt_info.state = BT_IDLE;
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:	// Unexpected data, treat OK+S like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:	// Unexpected data, treat OK+ST like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
				buff_putchar(&bluetooth_device.input_buffer, 'A');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:	// Unexpected data, treat OK+STA like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
				buff_putchar(&bluetooth_device.input_buffer, 'A');
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
				buff_putchar(&bluetooth_device.input_buffer, 'A');
				buff_putchar(&bluetooth_device.input_buffer, 'R');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:	// Unexpected data, treat OK+STAR like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'T');
				buff_putchar(&bluetooth_device.input_buffer, 'A');
				buff_putchar(&bluetooth_device.input_buffer, 'R');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_START_05:
			switch (newchar) {
			case '\n':
			case '\r':	// Recieved OK+START, module started
				bt_info.state = BT_IDLE;
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
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'e');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:	// Unexpected data, treat OK+Se like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'e');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_Set_03:
			switch (newchar) {
			case ':':
				bt_info.state = BT_Set_04;
				break;
			case '\n':
			case '\r':	// Unexpected data, treat OK+Set like data
				bt_info.state = BT_IDLE;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'e');
				buff_putchar(&bluetooth_device.input_buffer, 't');
				buff_putchar(&bluetooth_device.input_buffer, '\n');
				break;
			default:	// Unexpected data, treat OK+Set like data
				bt_info.state = BT_DATA;
				buff_putchar(&bluetooth_device.input_buffer, 'O');
				buff_putchar(&bluetooth_device.input_buffer, 'K');
				buff_putchar(&bluetooth_device.input_buffer, '+');
				buff_putchar(&bluetooth_device.input_buffer, 'S');
				buff_putchar(&bluetooth_device.input_buffer, 'e');
				buff_putchar(&bluetooth_device.input_buffer, 't');
				buff_putchar(&bluetooth_device.input_buffer, newchar);
				break;
			} 
			break;
		case BT_Set_04:

			switch (newchar)
			{
			case '\n':
			case '\r':
				bt_info.state = BT_IDLE;
				bt_info.data_len = 0;
				break;
			
			default:
				switch (bt_info.prevcmd) {
				case BTCMD_CHAR:
					if (bt_info.data_len < 4) {
						bt_info.chari[bt_info.data_len] = newchar;
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
					if (bt_info.data_len < 4) {
						bt_info.uuid[bt_info.data_len] = newchar;
					}
					break;
				default:
					break;
				}

				bt_info.data_len++;
				break;
			}
			
			break;
		
		default:
			bt_info.state = BT_IDLE;
			break;
		}
	}
}

io_error_t bluetooth_putchar(char c)
{
	return IO_NOT_IMPLMENT;
}

io_error_t bluetooth_getchar(char *c)
{
	return IO_NOT_IMPLMENT;
}