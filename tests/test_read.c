#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "../src/ssm2.h"

int main(void)
{
	unsigned int i = 0;
	unsigned char buf[32] = {0};
	int ret = 0;

	if (ssm2_open("/dev/ttyUSB0") != SSM2_ESUCCESS)
	{
		perror("[!] SSM2 open fail");
		exit(-1);
	}

	for (i = 0x00; i < 0xff; i++)
	{
		if ((ret = ssm2_ecu_read((unsigned int []) {i}, 1, buf)) != SSM2_ESUCCESS)
		{
			printf("Error: %s\n", ssm2_strerror(ret));
			ssm2_close();
			exit(-1);
		}

		printf("%02x\n", buf[0]);
	}
	ssm2_close();

	return 0;
}

