#include <stdio.h>
#include <unistd.h>

int main() {
    printf("WHY AIN'T IT WORKING?\n");
    
    while (1) {
        char i = getchar();
        printf("%02x\n", i);
    }

    return 0;
}