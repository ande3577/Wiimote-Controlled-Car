/*
 * comm.c
 *
 *  Created on: Sep 15, 2010
 *      Author: dsanderson
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdarg.h>

#include "control_board.h"
#include "timestamp.h"
#include "error_message.h"

int32_t fd; // file descriptor for the port

#if _DEBUG
bool comm_trace = true;
#else
bool comm_trace = false;
#endif

char tx_buffer[256];
char rx_buffer[256];

bool get_comm_trace(void)
{
	return comm_trace;
}

void set_comm_trace(bool enabled)
{
	comm_trace = enabled;
}

int open_port(char *name)
{
#if !_DIAGNOSTIC_MODE
	fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		/*
		 * Could not open the port.
		 */
		if (comm_trace)
			printf("open_port: Unable to open %s - ", name);
	}
	else
		fcntl(fd, F_SETFL, 0);

	return (fd);
#else
	return 1;
#endif
}

int initport(int fd)
{
#if !_DIAGNOSTIC_MODE
	struct termios options;
	// Get the current options for the port...
	tcgetattr(fd, &options);
	// Set the baud rates to 115200...
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	// Enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	/* set input mode (non-canonical, no echo,...) */
	options.c_lflag = 0;

	options.c_cc[VMIN] = 0;
#if !_DEBUG
	options.c_cc[VTIME] = 10;
#else
	options.c_cc[VTIME] = 50;
#endif

	// Set the new options for the port...
	tcsetattr(fd, TCSANOW, &options);
	fd = 1;
	return fd;
#else
	return 1;
#endif
}

int32_t comm_init(char *port_name)
{
#if !_DIAGNOSTIC_MODE
	fd = open_port(port_name);
	if (0 < fd)
		initport(fd);
	return fd;
#else
	return 1;
#endif
}

int32_t comm_close(void)
{
#if !_DIAGNOSTIC_MODE
	return close(fd);
#else
	return 0;
#endif
}

int32_t comm_flush()
{
	return tcflush(fd, TCIOFLUSH);
}

int32_t comm_readline(char *bfr, int32_t count)
{
	int32_t bytes_read = 0;
	int32_t rx_count = 0;
	char last_byte;

	do
	{
		bytes_read = read(fd, bfr + rx_count, count - rx_count);
		rx_count += bytes_read;
		if (0 < rx_count)
			last_byte = bfr[rx_count - 1];
		else
		{
			last_byte = '\0';
			break;
		}

	} while ((0 < bytes_read) && (rx_count < count) && (last_byte != '\n'));

	if (0 < rx_count)
		bfr[rx_count] = '\0';

	if (last_byte != '\n')
		return ERR_READ;
	else
		return rx_count;
}

int32_t comm_writeline(char *bfr)
{
	int32_t ret_val;
	ret_val = write(fd, bfr, strlen(bfr));
	if (0 <= ret_val)
		ret_val = write(fd, "\n", strlen("\n"));
	return ret_val;
}

char temp_buffer[256] =
{ 0 };

int32_t comm_validate_response(char *response, char *send, char *parameters)
{
	if (!strlen(send))
		return ERR_NONE;

	if (!strlen(response))
		return ERR_COMM_TIMEOUT;

	if (strncmp(response, send, strlen(send)))
		return ERR_COMMAND_MISMATCH;

	strcpy(temp_buffer, response + strlen(send));
	if (strncmp(temp_buffer, ":OK", strlen(":OK")))
	{
		if (strncmp(temp_buffer, ":ERR", strlen(":ERR")))
			return ERR_INVALID_RESPONSE;

		strcpy(parameters, temp_buffer + strlen(":ERR"));
		/// \todo why can't i link this?
		return decode_error_response(parameters);
	}

	strcpy(parameters, temp_buffer + strlen(":OK"));

	return ERR_NONE;
}

int32_t comm_query(char *parameters, const char *fmt, ...)
{

	int32_t ret_val;
	va_list args;
	va_start(args, fmt);
	ret_val = vsprintf(tx_buffer, fmt, args);
	if (0 > ret_val)
		return ERR_UNKN;

	if (comm_trace)
		printf("@%u: << %s\n", get_tick_count(), tx_buffer);

#if !_DIAGNOSTIC_MODE
	ret_val = comm_writeline(tx_buffer);
	if (0 >= ret_val)
		return ERR_WRITE;

	ret_val = comm_readline(rx_buffer, sizeof(rx_buffer));
	if (0 >= ret_val)
	{
		if (comm_trace)
			printf("@%u: >> %s", get_tick_count(), rx_buffer);
		return ret_val;

	}
	if (comm_trace)
		printf("@%u: >> %s", get_tick_count(), rx_buffer);

	return comm_validate_response(rx_buffer, tx_buffer, parameters);
#else
	printf("%s\n", tx_buffer);
	return ERR_NONE;

#endif

}

