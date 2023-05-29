/* Assignment3 Application
File : main.c
Auth : neo7k@sogang.ac.kr */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include "../module/stopwatch.h"

#define MAX_DIGIT 4
#define STOPWATCH_DEVICE "/dev/stopwatch"

int ioctl_set_option(int, char*);
int ioctl_command(int, char*);

int main(void)
{
	int dev;
	char data[4];
	unsigned char ret_val;
	char command[2];

	// initial FND value
	memset(data,0x30,sizeof(data));
	//memset(command,0x30,sizeof(command));
	strcpy(command, "0");

    dev = open(STOPWATCH_DEVICE, O_RDWR);
    if (dev<0) {
        printf("log:stopwatch device open error : %s\n",STOPWATCH_DEVICE);
        exit(1);
    }
	else
		printf("< stopwatch device has been detected > \n");
	usleep(10000);

	ret_val = ioctl_set_option(dev, data);
    if(ret_val>-1) {
		ret_val = ioctl_command(dev, command);
	}
	else
		printf("\nret_val = %d\n", ret_val);
	
/*
    ret_val=write(dev,&data,4);	
    if(ret_val<0) {
        printf("log:stopwatch write Error!\n");
		exit(1);
    }
*/
/*
	sleep(1);

	retval=read(dev,&data,4);
	if(retval<0) {
		printf("Read Error!\n");
		return -1;
	}
	printf("Current FND Value : ");
	for(i=0;i<str_size;i++)	
		printf("%d",data[i]);
	printf("\n");
*/
	close(dev);

	return(0);
}

int ioctl_set_option(int fd, char *msg) {
	int ret_val;

	ret_val = ioctl(fd, SET_OPTION, msg);
	if(ret_val<0)
		printf("log:ioctl_set_option failed:%d\n", ret_val);

	return ret_val;
}
int ioctl_command(int fd, char* cmd) {
	int ret_val;
	
	ret_val = ioctl(fd, COMMAND, cmd);
	if(ret_val < 0)
		printf("log:ioctl_command failed:%d\n", ret_val);

	return ret_val;
}
