/*
 * comm.h
 *
 *  Created on: Sep 15, 2010
 *      Author: dsanderson
 */

#ifndef COMM_H_
#define COMM_H_

int32_t comm_flush();
int32_t comm_writeline(char *buffer);
int32_t comm_readline(char *bfr, int32_t count);
int32_t comm_validate_response(char *response, char *cmd, char *parameters);
int32_t comm_query(char *parameters, const char *fmt, ...);

#endif /* COMM_H_ */
