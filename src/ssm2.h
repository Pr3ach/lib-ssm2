#ifndef SSM2_H
#define SSM2_H
#include <unistd.h>
#include <termios.h>

#define DBG		/* FIXME: Comment out on prod build */

#define MAX_DATA 128
#define MAX_QUERY MAX_DATA+64
#define MAX_RESPONSE 512

/* SSM2 header constants */
#define SSM2_INIT 0x80
#define SRC_ECU 0x10
#define SRC_DIAG 0xf0
#define DST_DIAG SRC_DIAG
#define DST_ECU SRC_ECU
#define SSM2_CMD_READ 0xa8
#define SSM2_CMD_READBLOCK 0xa0

/* SSM2 related errors */
#define SSM2_ESUCCESS 0
#define SSM2_ETIMEOUT -1	/* Query timed out */
#define SSM2_EUNKN -2		/* Unkown error */
#define SSM2_ENOQUERY -3	/* Nothing to be done */
#define SSM2_EWRITE -4		/* Write error */
#define SSM2_EBADCS -5		/* Bad response checksum */
#define SSM2_EDST -6		/* Destination response mismatch */
#define SSM2_EOPEN -7		/* Serial port open fail */
#define SSM2_EGETTTY -8		/* Fail to retrieve current TTY settings */
#define SSM2_ESETTTY -9		/* Fail to set current TTY settings */
#define SSM2_ECLOSE -10		/* Fail to set current TTY settings */

#define SSM2_QUERY_TIMEOUT 700 /* in msec; 0.7s */

/* SSM2 query */
typedef struct ssm2_query
{
	size_t q_size;			/* total size in bytes */
	unsigned char q_raw[MAX_QUERY]; /* full query */
	unsigned int q_resp_len;	/* computed expected total response length in bytes */

} ssm2_query;

/* SSM2 response */
typedef struct ssm2_response
{
	size_t r_size;				/* total size in bytes */
	unsigned char r_raw[MAX_RESPONSE];	/* full response */
} ssm2_response;

int fd;
struct termios tios, old_tios;
ssm2_query *q;		/* Global var for query */
ssm2_response *r;	/* Global var for response */

int ssm2_open(char *device);
int ssm2_close(void);
int ssm2_ecu_read(unsigned int *addresses, size_t count, unsigned char *out);
int ssm2_ecu_readblock(unsigned int from_addr, unsigned char count, unsigned char *out);

void init_query(ssm2_query *q);
unsigned char get_checksum(ssm2_query *q);
void print_raw_query(ssm2_query *q);
void print_raw_response(ssm2_response *r);
int get_query_response(unsigned char *out);
unsigned char get_response_checksum(ssm2_response *r);
unsigned long long time_ms(void);

#endif
