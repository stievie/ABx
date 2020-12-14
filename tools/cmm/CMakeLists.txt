project (cmm CXX)

include_directories(../../Include/stb)

file(GLOB SOURCES
    cmm/*.cpp
    cmm/*.h
)

add_executable(
    cmm
    ${SOURCES}
)

target_link_libraries(cmm abscommon absmath)

install(TARGETS cmm
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
