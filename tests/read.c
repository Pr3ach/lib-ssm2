#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "../src/ssm2.h"

void err(int ssm2_err);

int main(void)
{
	unsigned int i = 0;
	unsigned char buf[32] = {0};
	int ret = 0;

	/******************************************/
	/*					  */
	/* XXX: Update this array to test	  */
	/*					  */
	/******************************************/
	unsigned int READ_ADDR[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

	if (ssm2_open("/dev/ttyUSB0") != SSM2_ESUCCESS)
		err(1);

	if ((ret = ssm2_ecu_read(READ_ADDR, sizeof(READ_ADDR)/sizeof(unsigned int), buf)) != SSM2_ESUCCESS)
		err(ret);

	for (i = 0; i < sizeof(READ_ADDR)/sizeof(unsigned int); i++)
		printf("%c", buf[i]);

	ssm2_close();

	return 0;
}

void err(int ssm2_err)
{
	if (ssm2_err <= 0)
		printf("Error: %s\n", ssm2_strerror(ssm2_err));
	else
		perror("perror");

	ssm2_close();
	exit(-1);
}
