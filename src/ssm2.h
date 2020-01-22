/*
 *  Copyright (C) 2019 - Preacher
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SSM2_H
#define SSM2_H
#include <unistd.h>
#include <termios.h>

//#define DBG		/* FIXME: Comment out */

#define MAX_DATA 128
#define MAX_QUERY MAX_DATA+64
#define MAX_RESPONSE 512

/* SSM2 header constants */
#define SSM2_INIT 0x80
#define SRC_ECU 0x10
#define SRC_DIAG 0xf0
#define DST_DIAG SRC_DIAG
#define DST_ECU SRC_ECU
#define SSM2_QUERY_CMD 0xa8
#define SSM2_BLOCKQUERY_CMD 0xa0

/* SSM2 related errors */
#define SSM2_ESUCCESS 0
#define SSM2_ETIMEOUT -1
#define SSM2_EUNKN -2		/* Unkown error */
#define SSM2_ENOQUERY -3	/* Nothing to be done */
#define SSM2_EWRITE -4		/* Write error */
#define SSM2_EPARTIAL -5	/* Partial response from ECU */
#define SSM2_EBADCS -6		/* Bad response checksum */
#define SSM2_EDST -7		/* Destination response mismatch */
#define SSM2_EOPEN -8		/* Serial port open fail */
#define SSM2_EGETTTY -9		/* Fail to retrieve current TTY settings */
#define SSM2_ESETTTY -10	/* Fail to set current TTY settings */

#define SSM2_QUERY_TIMEOUT 300000 /* in usec; 0.3s */

/* SSM2 query */
typedef struct ssm2_query
{
	size_t q_size;			/* total size in byte */
	unsigned char q_raw[MAX_QUERY]; /* full query */

} ssm2_query;

/* SSM2 response */
typedef struct ssm2_response
{
	size_t r_size;				/* total size in byte */
	unsigned char r_raw[MAX_RESPONSE];	/* full response */
	unsigned int r_discarded;		/* allows to discard echo/loopback response */
} ssm2_response;

int fd;
struct termios tios, old_tios;
ssm2_query *q;		/* Global var for query */
ssm2_response *r;	/* Global var for response */

int ssm2_open(char *device);
int ssm2_close(void);
int ssm2_query_ecu(unsigned int *addresses, size_t count, unsigned char *out);
int ssm2_blockquery_ecu(unsigned int from_addr, unsigned char count, unsigned char *out);

void init_query(ssm2_query *q);
void sig_io_handler(int status);
unsigned char get_checksum(ssm2_query *q);
void print_raw_query(ssm2_query *q);
void print_raw_response(ssm2_response *r);
int get_query_response(unsigned char *out, int count);

#endif
