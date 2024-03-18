enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> -f elf32 -F dwarf -g -o <OBJECT> <SOURCE>")

set(ARCH_NAME i386)
set(ARCH_BITS 32)

set(ARCH_MACRO_NAME I386)

execute_process(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtbegin.o OUTPUT_VARIABLE CRTBEGIN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -print-file-name=crtend.o   OUTPUT_VARIABLE CRTEND_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CRTI_PATH ${CMAKE_CURRENT_BINARY_DIR}/crti.o)
set(CRTN_PATH ${CMAKE_CURRENT_BINARY_DIR}/crtn.o)

execute_process(COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf32 -o ${CRTI_PATH} ${CMAKE_CURRENT_LIST_DIR}/crti.asm)
execute_process(COMMAND ${CMAKE_ASM_NASM_COMPILER} -f elf32 -o ${CRTN_PATH} ${CMAKE_CURRENT_LIST_DIR}/crtn.asm)

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <CMAKE_C_LINK_FLAGS> <FLAGS> <LINK_FLAGS> ${CRTI_PATH} ${CRTBEGIN_PATH} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${CRTEND_PATH} ${CRTN_PATH}")

file(GLOB_RECURSE ARCH_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/**.c
    ${CMAKE_CURRENT_LIST_DIR}/src/**.cpp
    ${CMAKE_CURRENT_LIST_DIR}/asm/**.asm
)

# /home/iliashdz/opt/cross/bin/i686-elf-g++ -fno-pie -m32 -ffreestanding -fno-exceptions -fno-rtti -g -nostdlib -n -T /home/iliashdz/ampersand/linker.ld /home/iliashdz/ampersand/build/crti.o /home/iliashdz/opt/cross/lib/gcc/i686-elf/14.0.0/crtbegin.o