#ifndef STDLIB_H
#define STDLIB_H

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

#ifdef __cplusplus
extern "C" {
#endif

void exit(int status);

#define _Exit exit
#define _exit exit

#ifdef __cplusplus
}
#endif

#endif // STDLIB_H