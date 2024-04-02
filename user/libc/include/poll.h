#ifndef POLL_H
#define POLL_H

typedef unsigned long nfds_t;

#define POLLIN     0b0000000000000011
#define POLLRDNORM 0b0000000000000001
#define POLLRDBAND 0b0000000000000010

#define POLLOUT    0b0000000000001100
#define POLLWRNORM 0b0000000000000100
#define POLLWRBAND 0b0000000000001000

#define POLLPRI    0b0000000000010000

#define POLLERR    0b0000000000100000
#define POLLHUP    0b0000000001000000
#define POLLNVAL   0b0000000010000000

#ifdef __cplusplus
extern "C" {
#endif

struct pollfd {
    int   fd;
    short events;
    short revents;
};

int poll(struct pollfd fds[], nfds_t nfds, int timeout);

#ifdef __cplusplus
}
#endif

#endif // POLL_H