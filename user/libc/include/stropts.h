#ifndef STROPTS_H
#define STROPTS_H

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

int ioctl(int fd, int request, ...);

#ifdef __cplusplus
}
#endif

#endif // STROPTS_H