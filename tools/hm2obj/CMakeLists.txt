project (hm2obj CXX)

include_directories(../../Include/stb)

file(GLOB SOURCES
    hm2obj/*.cpp
    hm2obj/*.h
)

add_executable(
    hm2obj
    ${SOURCES}
)

target_link_libraries(hm2obj abscommon absmath)

install(TARGETS hm2obj
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
