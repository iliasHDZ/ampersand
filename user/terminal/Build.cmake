set(PROG_NAME terminal)

file(GLOB_RECURSE C_SRCS   ${CMAKE_CURRENT_LIST_DIR}/**.c)
file(GLOB_RECURSE CXX_SRCS ${CMAKE_CURRENT_LIST_DIR}/**.cpp)

message(STATUS ${CXX_SRCS})

add_executable(${PROG_NAME} ${C_SRCS} ${CXX_SRCS})

target_ampersand(${PROG_NAME})

add_custom_command(TARGET ${PROG_NAME} POST_BUILD
    COMMAND cp $<TARGET_FILE:${PROG_NAME}> ${IMAGE_DIR}/bin/${PROG_NAME}
)