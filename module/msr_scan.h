#include "types.h"

#define MSR_DEVICE_NAME "msrs"
#define MSR_DEVICE_PATH "/dev/" MSR_DEVICE_NAME

#define MSR_IOCTL_MAGIC_NUMBER 0xc00l

#define MSR_IOCTL_CMD_TRIGGER_SCAN _IOW(MSR_IOCTL_MAGIC_NUMBER, 1, int)

