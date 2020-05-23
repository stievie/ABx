project (glew C)

file(GLOB GLEW_SOURCES      src/*.c include/GL/*.h)

add_library(
    glew
    ${GLEW_SOURCES}
)
