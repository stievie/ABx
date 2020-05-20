project (lz4 C)

file(GLOB_RECURSE LZ4_SOURCES      lib/*.c lib/*.h)

add_library(
    lz4
    ${LZ4_SOURCES}
)
