project (obj2hm CXX)

include_directories(../Include/stb)

file(GLOB SOURCES
    obj2hm/*.cpp
    obj2hm/*.h
)

add_executable(
    obj2hm
    ${SOURCES}
)

target_link_libraries(obj2hm abscommon absmath)

install(TARGETS obj2hm
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
