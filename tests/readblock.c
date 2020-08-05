#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "../src/ssm2.h"

void err(int ssm2_err);

int main(int argc, char **argv, char **envp)
{
	unsigned char buf[512] = {0};
	int i = 0;
	int ret = 0;

	/******************************************/
	/*					  */
	/* XXX: Update these values to test	  */
	/*					  */
	/******************************************/
	int READ_START = 0;
	int READ_LENGTH = 0x70;

	if (ssm2_open("/dev/ttyUSB0") != SSM2_ESUCCESS)
		err(1);

	if ((ret = ssm2_ecu_readblock(READ_START, READ_LENGTH, buf)) != SSM2_ESUCCESS)
		err(ret);

	for (i = 0; i < READ_LENGTH; i++)
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
