qemu-system-i386 -s -S -debugcon stdio -m 4G -vga std -drive format=raw,file=build/ampersand.iso -drive id=disk,file=image.img,format=raw,if=ide