set(LIBC_NAME c)
set(LIBC_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE C_SRCS   ${LIBC_DIR}/src/**.c)
file(GLOB_RECURSE CXX_SRCS ${LIBC_DIR}/src/**.cpp)

add_library(${LIBC_NAME} STATIC ${C_SRCS} ${CXX_SRCS})
target_include_directories(${LIBC_NAME} PUBLIC ${LIBC_DIR}/include)

function(target_ampersand object)
    target_link_libraries(${object} ${LIBC_NAME})
    target_include_directories(${object} PUBLIC ${LIBC_DIR}/include)
    set_target_properties(${object} PROPERTIES LINK_FLAGS "-nostdlib -n -T ${LIBC_DIR}/linker.ld")
endfunction()