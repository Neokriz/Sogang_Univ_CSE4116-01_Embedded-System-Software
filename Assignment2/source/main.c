#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "dev.h"


#define FPGA_DEV_DEVICE "/dev/dev_driver"

int ioctl_set_option(int, char*);

int main(int argc, char **argv) {
	unsigned char ret_val;
	char *timer_init; 
	char msg[13] = {0x00};
	int timer_interval, timer_cnt;
	int err, len, zero, symbol;
	int dev;
	int i;

	err = 0; zero = 0, symbol = 0;
	timer_init = NULL;
	
	if(argc != 4) {
		printf("[ERR]Number of arguments doesn't match.\n\n");
		printf("Usage example: ./app [TIMER_INTERVAL] [TIMER_CNT] [TIMER_INIT]\n");
		printf("Arguments range:\n");
		printf("\t\tTIMER_INTERVAL(HZ) - Integer between 1 and 100 (0.1 - 10 seconds).\n");
		printf("\t\tTIMER_CNT - Integer between 1 and 100.\n");
		printf("\t\tTIMER_INIT - 4 digit code made up of three zeros and one integer between 1 and 8 (0001 - 8000).\n");
		exit(1);
	}

	timer_interval = atoi(argv[1]);
	if((timer_interval<1) || (timer_interval>100)) {
		printf("[ERR]First argument is out of range!\n");
		printf("\t\tTIMER_INTERVAL value must be an integer between 1 and 100.\n");
		err = 1;
	}

	timer_cnt = atoi(argv[2]);
	if((timer_cnt<1) || (timer_cnt>100)) {
		printf("[ERR]Second argument is out of range!\n");
		printf("\t\tTIMER_CNT value must be an integer between 1 and 100.\n");
		err = 1;
	}
	len = strlen(argv[3]);
	timer_init = (char*)malloc(sizeof(char) * len);
	strcpy(timer_init, argv[3]);	
	for(i=0; i<len; ++i) {
		if(argv[3][i] == '0')
			zero++;
		else if((argv[3][i] < '0') || (argv[3][i] > '8'))
			symbol = -1;
	}

	if((len < 4) || (len > 4)) {
		printf("[ERR]Third argument invalid.\n");
		printf("\t\tTIMER_INIT must be 4 digit code.\n");
		err = 1;
	}
	else if(zero != 3) {
		printf("[ERR]Third argument invalid.\n");
		if(zero == 4)
			printf("\t\tNo symbol in code.\n");
		else
			printf("\t\tToo many symbols in code.\n");
		err = 1;
	}
	else if(symbol < 0) {
		printf("[ERR]Third argument invalid.\n");
		printf("Symbol must be an integer between 1 and 8.\n");
		err = 1;
	}

	if(err == 1)
		exit(1);
	else {
		printf("-------------------------------------------\n");
		printf("Arguments confirmed: %3d %3d %s\n", timer_interval, timer_cnt, timer_init);
		sprintf(msg, "%3d %3d %s", timer_interval, timer_cnt, timer_init);
		printf("-------------------------------------------\n");
	}

	printf("log:device open\n");
	//dev = open(FPGA_DEV_DEVICE, O_WRONLY);
	dev = open(FPGA_DEV_DEVICE, O_RDWR);
	if(dev<0) {
		printf("Device open error : %s\n", FPGA_DEV_DEVICE);
		exit(1);
	}

	printf("log:write call\n");
	//ret_val = write(dev, timer_init, 4);
	ret_val = ioctl_set_option(dev, msg);
	
	printf("log:write called\n");

	close(dev);

	free(timer_init);

	return ret_val;
}

int ioctl_set_option(int fd, char *message) {
	int ret_val;
	
	ret_val = ioctl(fd, SET_OPTION, message);

	printf("log:ioctl_set_option [1]message:%s\n", message);

	if(ret_val < 0)
		printf("log:ioctl_set_option failed:%d\n", ret_val);

	return ret_val;
}
