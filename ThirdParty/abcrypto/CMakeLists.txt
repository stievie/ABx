cmake_minimum_required (VERSION 3.17)
project (abcrypto C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-sign -Wno-comment -Wno-missing-braces -Wno-missing-field-initializers")
endif()

# CPP/HPP files are just wrapper, not part of the library.
file(GLOB_RECURSE ABCRYPTO_SOURCES      ./*.c ./*.h)

add_library(
    abcrypto
    ${ABCRYPTO_SOURCES}
)
