#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "../src/ssm2.h"

int main(void)
{
	unsigned int pos = 0;
	unsigned int i  = 0;
	unsigned char buf[0x80] = {0};
	int ret = 0;

	if (ssm2_open("/dev/ttyUSB0") != SSM2_ESUCCESS)
	{
		perror("[!] SSM2 open fail");
		exit(-1);
	}

	for (pos = 0x00; pos < 0xffff7f; pos += 0x80)
	{
		if ((ret = ssm2_ecu_readblock(pos, 0x80, buf)) != SSM2_ESUCCESS)
		{
			printf("Error: %s\n", ssm2_strerror(ret));
			ssm2_close();
			exit(-1);
		}

		for (i = 0; i < 0x80; i++)
			printf("%c", buf[i]);
		fflush(stdout);
	}
	ssm2_close();

	return 0;
}

