#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "ssm2.h"

int main(void)
{
	int i = 0;
	unsigned char buf[32] = {0};
	int ret = 0;

	if (ssm2_open("/dev/ttyUSB0") != SSM2_ESUCCESS)
	{
		perror("[!] SSM2 open fail");
		exit(-1);
	}

	for (i = 0x01; i < 0xffff; i++)
	{
		if ((ret = ssm2_query_ecu((unsigned int []) {i}, buf, 1)) != SSM2_ESUCCESS)
		{
			printf("[!] ret val: %d\n", ret);
			ssm2_close();
			exit(-1);
		}

		printf("%c", buf[0]);
	}
	ssm2_close();

	return 0;
}

