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

	printf("a\n");
	if ((ret = ssm2_blockquery_ecu(1, 0xff, buf)) != SSM2_ESUCCESS)
	{
		perror("ssm2_blockread");
		printf("ret: %d\n", ret);
		ssm2_close();
		exit(1);
	}
	printf("b\n");

	for (i = 0; i < 0xff; i++)
	{
		if (!(i%16))
			printf("%02x \n", buf[i]);
		else
			printf("%02x ", buf[i]);
	}
	ssm2_close();

	return 0;
}
