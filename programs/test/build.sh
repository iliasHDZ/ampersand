#!/bin/bash

CC=~/opt/cross/bin/i686-elf-gcc

${CC} -Wall -fno-pie -m32 -ffreestanding -c -o main.o main.c

${CC} -nostdlib -n -T linker.ld -o test main.o