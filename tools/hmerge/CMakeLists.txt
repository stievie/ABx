project (hmerge CXX)

include_directories(../../Include/stb)

file(GLOB SOURCES
    hmerge/*.cpp
    hmerge/*.h
)

add_executable(
    hmerge
    ${SOURCES}
)

target_link_libraries(hmerge abscommon absmath)

install(TARGETS hmerge
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
