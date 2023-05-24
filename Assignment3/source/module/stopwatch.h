#ifndef STOPWATCH_H_
#define STOPOWATCh_H_
#include <asm/ioctl.h>

#define MAJOR_NUM 242
#define DEV_NAME "stopwatch"

#define SET_OPTION _IOW(MAJOR_NUM, 0, char*)
#define COMMAND _IOW(MAJOR_NUM, 1, char*)

#endif
