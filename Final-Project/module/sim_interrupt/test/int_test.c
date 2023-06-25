#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
int main(void) {
	int fd;
	int retn;
	char buf[2] = {0,0};

	fd = open("/dev/sim_interrupt", O_RDWR);
	if(fd < 0) {
		perror("/dev/sim_interrupt error");
		exit(-1);
	}
    else { 
		printf("< inter Device has been detected > \n"); 
	}
	
	retn = read(fd, buf, 2);
	close(fd);

	return 0;
}

