cmake_minimum_required(VERSION 3.27.7)

set(PROJECT_NAME ampersand)
set(AMPERSAND_VERSION 1.0.0)

project(${PROJECT_NAME} VERSION ${AMPERSAND_VERSION} LANGUAGES C CXX ASM_NASM)

###############################
####  KERNEL ARCH LIBRARY  ####
###############################

if("${TARGET_ARCH}" STREQUAL i386)
    include(./kernel/arch/i386/Arch.cmake)
else()
    message(FATAL_ERROR "Ampersand kernel does not support architecture '${TARGET_ARCH}'")
endif()

##################
####  KERNEL  ####
##################

set(CMAKE_BUILD_TYPE Debug)
set(UNIT_TESTING_BEFORE_STARTUP 1)

set(CMAKE_C_FLAGS   "-Wall -fno-pie -m32 -ffreestanding")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fno-rtti")

file(GLOB_RECURSE C_SRCS   ./kernel/src/**.c)
file(GLOB_RECURSE CXX_SRCS ./kernel/src/**.cpp)

file(GLOB_RECURSE ARCH_COMMON_C_SRCS   ./kernel/arch/common/**.c)
file(GLOB_RECURSE ARCH_COMMON_CXX_SRCS ./kernel/arch/common/**.cpp)

if(${UNIT_TESTING_BEFORE_STARTUP} EQUAL 1)
    file(GLOB_RECURSE TEST_CXX_SRCS ./test/**.cpp)
else()
    set(TEST_CXX_SRCS "")
endif()

add_executable(kernel ${C_SRCS} ${CXX_SRCS} ${ARCH_SRCS} ${TEST_CXX_SRCS} ${ARCH_COMMON_C_SRCS} ${ARCH_COMMON_CXX_SRCS})

add_compile_definitions(kernel PRIVATE ARCH_NAME="${ARCH_NAME}")
add_compile_definitions(kernel PRIVATE ARCH_BITS=${ARCH_BITS})
add_compile_definitions(kernel PRIVATE "ARCH_${ARCH_MACRO_NAME}")
add_compile_definitions(kernel PRIVATE AMPERSAND_VERSION="${AMPERSAND_VERSION}")
if(${UNIT_TESTING_BEFORE_STARTUP} EQUAL 1)
    add_compile_definitions(kernel PRIVATE UNIT_TESTING_BEFORE_STARTUP)
endif()

target_include_directories(kernel PUBLIC ./kernel)
target_include_directories(kernel PUBLIC ./kernel/src)
target_include_directories(kernel PUBLIC ./user/libc/include)

set_target_properties(kernel PROPERTIES LINK_FLAGS   "-nostdlib -n -T ${CMAKE_SOURCE_DIR}/linker.ld")

#########################
####  USER PROGRAMS  ####
#########################

set(IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/image)

include(./user/libc/Build.cmake)

include(./user/terminal/Build.cmake)
include(./user/bash/Build.cmake)