cmake_minimum_required (VERSION 3.17)
project (abcrypto C)

# CPP/HPP files are just wrapper, not part of the library.
file(GLOB_RECURSE ABCRYPTO_SOURCES      ./*.c ./*.h)

add_library(
    abcrypto
    ${ABCRYPTO_SOURCES}
)
