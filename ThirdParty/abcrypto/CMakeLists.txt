project (abcrypto C)

# CPP/HPP files are just wrapper, not part of the library.
file(GLOB ABCRYPTO_SOURCES      ./*.c ./*.h)

add_library(
    abcrypto
    ${ABCRYPTO_SOURCES}
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(abcrypto PRIVATE -Wno-pointer-sign -Wno-comment -Wno-missing-braces -Wno-missing-field-initializers -Wformat-truncation=0)
elseif(MSVC)
    target_compile_options(abcrypto PRIVATE /wd4201 /wd4244 /wd4057 /wd4295)
endif()
