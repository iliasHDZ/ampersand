int _start() {
    for (int i = 0; i < 10; i++)
        asm("int $0xC0");

    for (;;);
}