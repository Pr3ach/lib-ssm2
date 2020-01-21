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


/* SSM2 header: */
/*unsigned char init;		init byte (0x80) */
/*unsigned char dst;		destination byte. ECU (0x10) or Diag tool (0xf0) */
/*unsigned char src;		source byte. Same as above */
/*unsigned char size;		data size in bytes */
/*unsigned char data[MAX_DATA];	actual data */
/*unsigned char checksum;	checksum */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include "ssm2.h"

/*
 *
 * Open specified serial device for read/write.
 * Return < 0 if error, 0 otherwise.
 *
 */
int ssm2_open(char *device)
{
	if ((fd = open(device, O_RDWR | O_NOCTTY)) < 0)
		return SSM2_EOPEN;

	/* get current TTY settings */
	if (tcgetattr(fd, &tios) < 0)
	{
		close(fd);
		return SSM2_EGETTTY;
	}

	/* save options so they can be restored later */
	old_tios = tios;

	/* set 4800 baud in and out */
	cfsetspeed(&tios, B4800);
	/* set raw mode */
	cfmakeraw(&tios);

	/*
	 * 8N1: 1 stop bit, no flow ctl, 8 data bits, no parity
	 */
	tios.c_cflag &= ~CSTOPB;
	tios.c_cflag |= CLOCAL;
	tios.c_cflag |= CS8;
	tios.c_cflag &= ~PARENB;

	tcflush(fd, TCIFLUSH);

	/* Apply new TTY options */
	if (tcsetattr(fd, TCSANOW, &tios) < 0)
	{
		close(fd);
		return SSM2_ESETTTY;
	}

	q = calloc(1, sizeof(ssm2_query));
	r = calloc(1, sizeof(ssm2_response));

	return SSM2_ESUCCESS;
}

/*
 * Query ECU for data at specified addresses locations.
 *
 * addresses: int array of ECU addresses to query
 * out: where to store the results
 * count: number of addresses to query
 *
 * Return SSM2_ESUCESS on success
 *
 */
int ssm2_query_ecu(unsigned int *addresses, size_t_count, unsigned char *out)
{
	size_t i = 0;
	int c = 0;

	if (count < 1)
		return SSM2_ENOQUERY;

	init_query(q);
	memset(r, 0, sizeof(ssm2_response));

	/* size = command + pad + addresses = 1 + 1 + 3*count */
	q->q_raw[3] = 3*count + 2;

	/* data payload */
	q->q_raw[4] = (unsigned char) SSM2_QUERY_CMD;
	q->q_raw[5] = (unsigned char) 0;	/* pad byte */
	for (i = 0, c = 6; i < count; i++, c++)
	{
		q->q_raw[c++] = addresses[i] & 0xff0000;
		q->q_raw[c++] = addresses[i] & 0x00ff00;
		q->q_raw[c] = addresses[i] & 0x0000ff;
	}
	q->q_size = c + 1;
	q->q_raw[c] = get_checksum(q);

#ifdef DBG
	print_raw_query(q);
#endif

	if (write(fd, q->q_raw, q->q_size) != (ssize_t) q->q_size)
		return SSM2_EWRITE;

	return get_query_response(out, count);
}

/*
 * Query ECU for count bytes from address from_addr
 *
 * from_addr: address from where to start reading
 * count: number of bytes to read from from_addr address
 * buf: store results here
 *
 * Return SSM2_ESUCESS on success
 */
int ssm2_blockquery_ecu(unsigned int from_addr, unsigned char count, unsigned char *out)
{
	if (count <= 0 || from_addr == 0)
		return SSM2_ENOQUERY;
	count--;

	init_query(q);
	memset(r, 0, sizeof(ssm2_response));

	q->q_raw[3] = 6;
	q->q_raw[4] = SSM2_BLOCKQUERY_ECU;
	q->q_raw[5] = (unsigned char) 0;	/* pad byte */
	q->q_raw[6] = from_addr & 0x0000ff;
	q->q_raw[7] = from_addr & 0x00ff00;
	q->q_raw[8] = from_addr & 0xff0000;
	q->q_raw[9] = count;
	q->q_size = 11;
	q->q_raw[10] = get_checksum(q);

	if (write(fd, q->q_raw, q->q_size) != (ssize_t) q->q_size)
		return SSM2_EWRITE;

	return get_query_response(out, count);
}

/*
 * Initialize SSM2 header
 *
 */
void init_query(ssm2_query *q)
{
	memset(q, 0, sizeof(ssm2_query));

	q->q_raw[0] = SSM2_INIT;
	q->q_raw[1] = DST_ECU;
	q->q_raw[2] = SRC_DIAG;
}

/*
 * Print raw ssm2 query, for debug purposes
 */
void print_raw_query(ssm2_query *q)
{
	size_t i = 0;

	printf("raw query   : ");
	for (i = 0; i < 32; i++)
		printf("%02x ", q->q_raw[i]);
	printf("\n");
}

/*
 * Print raw ssm2 response, for debug purposes
 */
void print_raw_response(ssm2_response *r)
{
	size_t i = 0;

	printf("raw response: ");
	for (i = 0; i < 32; i++)
		printf("%02x ", r->r_raw[i]);
	printf("\n");
}

/*
 * Compute and return ssm2 response checksum.
 *
 */
unsigned char get_response_checksum(ssm2_response *r)
{
	unsigned char ck = 0;
	size_t i = 0;

	for (i = q->q_size; i < r->r_size-1; ck += r->r_raw[i++]);

	return ck;
}

/*
 * Compute and return ssm2 query checksum.
 *
 */
unsigned char get_checksum(ssm2_query *q)
{
	unsigned char ck = 0;
	size_t i = 0;

	for (i = 0; i < q->q_size; ck += q->q_raw[i++]);

	return ck;
}

/*
 * Read and update response buffer and out.
 * Return 0 on success, < 0 otherwise.
 *
 */
int get_query_response(unsigned char *out, int count)
{
	unsigned int bytes_avail = 0;
	clock_t start = clock();

	do
	{
		ioctl(fd, FIONREAD, &bytes_avail);
	} while(bytes_avail < q->q_size + 7 && start - clock() < SSM2_QUERY_TIMEOUT);

	if ((r->r_size = read(fd, r->r_raw, MAX_RESPONSE-1)) < q->q_size + 7)
		return SSM2_EPARTIAL;

#ifdef DBG
	print_raw_response(r);
#endif

	if (r->r_raw[q->q_size+1] != DST_DIAG)
		return SSM2_EDST;

	if (get_response_checksum(r) != r->r_raw[r->r_size-1])
		return SSM2_EBADCS; /* checksum mismatch */

	/* discard loopback */
	memcpy(out, r->r_raw+(q->q_size+5), r->r_raw[3] - 1);

	return SSM2_ESUCCESS;
}

/*
 * Unset signal handler, clean TTY, set old TTY options and close device.
 * Return > 0 if any error happens, 0 otherwise.
 *
 */
int ssm2_close(void)
{
	int ret_mask = 0;

	/* restore old TTY settings */
	tcflush(fd, TCIFLUSH);
	if (tcsetattr(fd, TCSANOW, &old_tios) != 0)
		ret_mask |= 1;

	/* close FD */
	if (close(fd) != 0)
		ret_mask |= 2;

	if (r)
		free(r);
	if (q)
		free(q);

	return ret_mask;
}

