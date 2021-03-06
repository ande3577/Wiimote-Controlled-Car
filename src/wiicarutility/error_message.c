/*
 * error_message.c
 *
 *  Created on: Dec 22, 2010
 *      Author: desertfx5
 */

#include <string.h>

#include "error_message.h"

/// \todo need to update this with controller board
int32_t decode_error_response(char *response)
{
	if (!strncmp(response, "PARAM", strlen("PARAM")))
		return ERR_PARAM;

	if (!strncmp(response, "CMD", strlen("CMD")))
		return ERR_CMD;

	if (!strncmp(response, "EXEC", strlen("EXEC")))
		return ERR_EXEC;

	if (!strncmp(response, "MTO", strlen("MTO")))
		return ERR_MOTOR_TIMEOUT;

	if (!strncmp(response, "NO_ERR", strlen("NO_ERR")))
		return ERR_NONE;

	if (!strncmp(response, "FRAME", strlen("FRAME")))
		return ERR_FRAME;

	return ERR_UNKN;
}

void format_error_string(ErrorID_t error_code, char *error_string)
{
	switch (error_code)
	{
	case ERR_NONE:
		strcpy(error_string, "No error");
		break;
	case ERR_MOTOR_TIMEOUT:
		strcpy(error_string, "Motor timeout");
		break;
	case ERR_PARAM:
		strcpy(error_string, "Invalid parameter(s)");
		break;
	case ERR_CMD:
		strcpy(error_string, "Unrecognized Command");
		break;
	case ERR_EXEC:
		strcpy(error_string, "Execution Error");
		break;
	case ERR_BUFFER_FULL:
		strcpy(error_string, "A buffer was full");
		break;
	case ERR_BUFFER_EMPTY:
		strcpy(error_string, "A buffer was empty");
		break;
	case ERR_COMMAND_MISMATCH:
		strcpy(error_string, "Sensor reply mismatch");
		break;
	case ERR_COMM_TIMEOUT:
		strcpy(error_string, "COMM Timeout");
		break;
	case ERR_INVALID_RESPONSE:
		strcpy(error_string, "Invalid Response");
		break;
	case ERR_WRITE:
		strcpy(error_string, "Write error");
		break;
	case ERR_READ:
		strcpy(error_string, "Read Error");
		break;
	case ERR_FRAME:
		strcpy(error_string, "Read Error");
		break;
	case ERR_PORT_INIT:
		strcpy(error_string, "Could not open comm port");
		break;
	default:
		strcpy(error_string, "Unknown error");
		break;
	}
}
