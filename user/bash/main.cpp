#include <unistd.h>
#include <fcntl.h>

int main() {
    write(0, "testings! \e[91mRED!", 20);

    for (;;);
}