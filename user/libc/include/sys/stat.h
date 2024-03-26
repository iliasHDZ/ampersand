#ifndef STAT_H
#define STAT_H

#define S_ISUID (0x800)
#define S_ISGID (0x400)
#define S_ISVTX (0x200)

// - r-- --- ---
#define S_IRUSR (0x100)
// - -w- --- ---
#define S_IWUSR (0x080)
// - --x --- ---
#define S_IXUSR (0x040)

// - --- r-- ---
#define S_IRGRP (0x020)
// - --- -w- ---
#define S_IWGRP (0x010)
// - --- --x ---
#define S_IXGRP (0x008)

// - --- --- r--
#define S_IROTH (0x004)
// - --- --- -w-
#define S_IWOTH (0x002)
// - --- --- --x
#define S_IXOTH (0x001)

// - rwx --- ---
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
// - --- rwx ---
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
// - --- --- rwx
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)

#define S_IFMT (0xf000)

// Block Device
#define S_IFBLK  (0x6000)
// Character Device
#define S_IFCHR  (0x2000)
// FIFO
#define S_IFIFO  (0x1000)
// Regular File
#define S_IFREG  (0x8000)
// Directory
#define S_IFDIR  (0x4000)
// Symbolic Link
#define S_IFLNK  (0xA000)
// Socket
#define S_IFSOCK (0xC000)

#define S_ISBLK(M)  ( (M & S_IFMT) == S_IFBLK )
#define S_ISCHR(M)  ( (M & S_IFMT) == S_IFCHR )
#define S_ISDIR(M)  ( (M & S_IFMT) == S_IFDIR )
#define S_ISFIFO(M) ( (M & S_IFMT) == S_IFIFO )
#define S_ISREG(M)  ( (M & S_IFMT) == S_IFREG )
#define S_ISLNK(M)  ( (M & S_IFMT) == S_IFLNK )
#define S_ISSOCK(M) ( (M & S_IFMT) == S_IFSOCK )

#endif // STAT_H