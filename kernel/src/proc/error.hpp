#pragma once

#include <common.h>

// No error
#define ENOERR          0x00
// Argument list too long
#define E2BIG           0x01
// Permission denied
#define EACCES          0x02
// Address in use
#define EADDRINUSE      0x03
// Address not available
#define EADDRNOTAVAIL   0x04
// Address family not supported
#define EAFNOSUPPORT    0x05
// Resource unavailable, try again
#define EAGAIN          0x06
// Connection already in progress
#define EALREADY        0x07
// Bad file descriptor
#define EBADF           0x08
// Bad message
#define EBADMSG         0x09
// Device or resource busy
#define EBUSY           0x0a
// Operation canceled
#define ECANCELED       0x0b
// No child processes
#define ECHILD          0x0c
// Connection aborted
#define ECONNABORTED    0x0d
// Connection refused
#define ECONNREFUSED    0x0e
// Connection reset
#define ECONNRESET      0x0f
// Resource deadlock would occur
#define EDEADLK         0x10
// Destination address required
#define EDESTADDRREQ    0x11
// Mathematics argument out of domain of function
#define EDOM            0x12
// Reserved
#define EDQUOT          0x13
// File exists
#define EEXIST          0x14
// Bad address
#define EFAULT          0x15
// File too large
#define EFBIG           0x16
// Host is unreachable
#define EHOSTUNREACH    0x17
// Identifier removed
#define EIDRM           0x18
// Illegal byte sequence
#define EILSEQ          0x19
// Operation in progress
#define EINPROGRESS     0x1a
// Interrupted function
#define EINTR           0x1b
// Invalid argument
#define EINVAL          0x1c
// I/O error
#define EIO             0x1d
// Socket is connected
#define EISCONN         0x1e
// Is a directory
#define EISDIR          0x1f
// Too many levels of symbolic links
#define ELOOP           0x20
// File descriptor value too large
#define EMFILE          0x21
// Too many links
#define EMLINK          0x22
// Message too large
#define EMSGSIZE        0x23
// Reserved
#define EMULTIHOP       0x24
// Filename too long
#define ENAMETOOLONG    0x25
// Network is down
#define ENETDOWN        0x26
// Connection aborted by network
#define ENETRESET       0x27
// Network unreachable
#define ENETUNREACH     0x28
// Too many files open in system
#define ENFILE          0x29
// No buffer space available
#define ENOBUFS         0x2a
// No message is available on the STREAM head read queue
#define ENODATA         0x2b
// No such device
#define ENODEV          0x2c
// No such file or directory
#define ENOENT          0x2d
// Executable file format error
#define ENOEXEC         0x2e
// No locks available
#define ENOLCK          0x2f
// Reserved
#define ENOLINK         0x30
// Not enough space
#define ENOMEM          0x31
// No message of the desired type
#define ENOMSG          0x32
// Protocol not available
#define ENOPROTOOPT     0x33
// No space left on device
#define ENOSPC          0x34
// No STREAM resources
#define ENOSR           0x35
// Not a STREAM
#define ENOSTR          0x36
// Functionality not supported
#define ENOSYS          0x37
// The socket is not connected
#define ENOTCONN        0x38
// Not a directory
#define ENOTDIR         0x39
// Directory not empty
#define ENOTEMPTY       0x3a
// State not recoverable
#define ENOTRECOVERABLE 0x3b
// Not a socket
#define ENOTSOCK        0x3c
// Not supported
#define ENOTSUP         0x3d
// Inappropriate I/O control operation
#define ENOTTY          0x3e
// No such device or address
#define ENXIO           0x3f
// Operation not supported on socket
#define EOPNOTSUPP      0x40
// Value too large to be stored in data type
#define EOVERFLOW       0x41
// Previous owner died
#define EOWNERDEAD      0x42
// Operation not permitted
#define EPERM           0x43
// Broken pipe
#define EPIPE           0x44
// Protocol error
#define EPROTO          0x45
// Protocol not supported
#define EPROTONOSUPPORT 0x46
// Protocol wrong type for socket
#define EPROTOTYPE      0x47
// Result too large
#define ERANGE          0x48
// Read-only file system
#define EROFS           0x49
// Invalid seek
#define ESPIPE          0x4a
// No such process
#define ESRCH           0x4b
// Reserved
#define ESTALE          0x4c
// Stream ioctl() timeout
#define ETIME           0x4d
// Connection timed out
#define ETIMEDOUT       0x4e
// Text file busy
#define ETXTBSY         0x4f
// Operation would block
#define EWOULDBLOCK     0x50
// Cross-device link
#define EXDEV           0x51


// Invalid file system type
#define EINVFS          0x80

typedef u32 SyscallError;