project (abcrypto C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-pointer-sign -Wno-comment -Wno-missing-braces -Wno-missing-field-initializers -Wformat-truncation=0)
endif()

# CPP/HPP files are just wrapper, not part of the library.
file(GLOB_RECURSE ABCRYPTO_SOURCES      ./*.c ./*.h)

add_library(
    abcrypto
    ${ABCRYPTO_SOURCES}
)
