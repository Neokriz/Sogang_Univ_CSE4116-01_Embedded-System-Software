#ifndef DEV_H_
#define DEF_H_
#include <asm/ioctl.h>

#define MAJOR_NUM 242
#define SET_OPTION _IOW(MAJOR_NUM, 0, char*)
#define COMMAND _IOW(MAJOR_NUM, 1, int)

#endif
