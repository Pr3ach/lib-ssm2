/*
 * Subaru Select Monitor 2 (SSM2) library
 *
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
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include "ssm2.h"

/*
 * Open specified serial device for read/write.
 *
 * device: serial device connected to ECU
 *
 * Return SSM2_ESUCCESS on success
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
 * count: number of addresses to query
 * out: where to store the results
 *
 * Return SSM2_ESUCESS on success
 *
 */
int ssm2_query_ecu(unsigned int *addresses, size_t count, unsigned char *out)
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
	for (i = 0, c = 6; i < count; i++, c += 3)
	{
		q->q_raw[c+2] = (addresses[i] & 0xff);
		q->q_raw[c+1] = (addresses[i]>>8 & 0xff);
		q->q_raw[c] = (addresses[i]>>16 & 0xff);
	}
	q->q_size = c + 1;
	q->q_raw[c] = get_checksum(q);

#ifdef DBG
	print_raw_query(q);
#endif

	if (write(fd, q->q_raw, q->q_size) != (ssize_t) q->q_size)
		return SSM2_EWRITE;

	return get_query_response(out);
}

/*
 * Query ECU for count bytes from address from_addr
 *
 * from_addr: address from where to start reading
 * count: number of bytes to read from from_addr address
 * out: store results here
 *
 * Return SSM2_ESUCESS on success
 */
int ssm2_blockquery_ecu(unsigned int from_addr, unsigned char count, unsigned char *out)
{
	if (count == 0 || from_addr + count > 0xffffff)
		return SSM2_ENOQUERY;
	count--; /* ECU will return count+1 bytes */

	init_query(q);
	memset(r, 0, sizeof(ssm2_response));

	q->q_raw[3] = 6; /* data size = cmd + pad + addr + count = 6 bytes */
	q->q_raw[4] = SSM2_BLOCKQUERY_CMD;
	q->q_raw[5] = (unsigned char) 0;	/* pad byte */
	q->q_raw[6] = (from_addr>>16) & 0xff;
	q->q_raw[7] = (from_addr>>8) & 0xff;
	q->q_raw[8] = from_addr & 0xff;
	q->q_raw[9] = count;
	q->q_size = 11;
	q->q_raw[10] = get_checksum(q);

#ifdef DBG
	print_raw_query(q);
#endif

	if (write(fd, q->q_raw, q->q_size) != (ssize_t) q->q_size)
		return SSM2_EWRITE;

	return get_query_response(out);
}

/*
 * Read and update response buffer and out.
 *
 * out: Pointer to a buffer to receive ECU response
 *
 * Return SSM2_ESUCESS on success
 *
 */
int get_query_response(unsigned char *out)
{
	unsigned int bytes_avail = 0;
	unsigned long long start_time = time_ms();

	do
	{
		ioctl(fd, FIONREAD, &bytes_avail);
	} while(bytes_avail < (q->q_size + 7) && time_ms() - start_time < SSM2_QUERY_TIMEOUT);

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
 * Clean TTY, set back old TTY options and close device.
 *
 * Return either SSM2_ESUCCESS on success, or a mask of SSM2_ECLOSE|SSM2_ESETTTY
 *
 */
int ssm2_close(void)
{
	int ret_mask = SSM2_ESUCCESS;

	/* restore old TTY settings */
	tcflush(fd, TCIFLUSH);
	if (tcsetattr(fd, TCSANOW, &old_tios) != 0)
		ret_mask = SSM2_ESETTTY;

	/* close FD */
	if (close(fd) != 0)
		ret_mask |= SSM2_ECLOSE;

	if (r)
		free(r);
	if (q)
		free(q);

	return ret_mask;
}

/*
 * Initialize SSM2 query header.
 *
 * q: Pointer to a ssm2_query structure
 */
void init_query(ssm2_query *q)
{
	memset(q, 0, sizeof(ssm2_query));

	q->q_raw[0] = SSM2_INIT;
	q->q_raw[1] = DST_ECU;
	q->q_raw[2] = SRC_DIAG;
}

/*
 * Print raw ssm2 query, for debug purposes.
 *
 * q: Pointer to a ssm2_query structure
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
 * Print raw ssm2 response, for debug purposes.
 *
 * r: Pointer to a ssm2_response structure
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
 * r: Pointer to a ssm2_response structure
 *
 * Return response checksum
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
 * q: Pointer to a ssm2_query structure
 *
 * Return query checksum
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
 * Get time of day in milliseconds
 *
 * Return time of day in milliseconds
 */
unsigned long long time_ms(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (unsigned long long) tv.tv_sec * 1000 + (unsigned long long) tv.tv_usec / 1000;
}
