#ifndef WAIT_H
#define WAIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int pid_t;

pid_t waitpid(pid_t pid, int* stat_loc, int options);

#ifdef __cplusplus
}
#endif

#endif // WAIT_H