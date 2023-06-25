#ifndef SIM_DRIVER_H_
#define SIM_DRIVER_H_
#include <asm/ioctl.h>

#define MAJOR_NUM 242
#define DEV_NAME "sim_driver"

#define SET_DATA _IOW(MAJOR_NUM, 0, char*)
#define COMMAND _IOW(MAJOR_NUM, 1, char*)

#endif
