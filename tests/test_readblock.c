#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "../src/ssm2.h"

int main(int argc, char **argv, char **envp)
{
	unsigned char buf[512] = {0};
	int i = 0;
	int ret = 0;

	if (ssm2_open("/dev/ttyUSB0") != SSM2_ESUCCESS)
	{
		perror("ssm2_open");
		exit(1);
	}

	if ((ret = ssm2_ecu_readblock(0x000000, 0x80, buf)) != SSM2_ESUCCESS)
	{
		printf("Error: %s\n", ssm2_strerror(ret));
		ssm2_close();
		exit(1);
	}

	printf("Response:\n");
	for (i = 0; i < 0x80; i++)
		printf("%02x%c", buf[i], !((i+1)%16) ? '\n' : ' ');
	printf("\n");
	ssm2_close();

	return 0;
}
