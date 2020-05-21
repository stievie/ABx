project (dbtool CXX)

file(GLOB DBTOOL_SOURCES
    dbtool/*.cpp
    dbtool/*.h
)

add_executable(
    dbtool
    ${DBTOOL_SOURCES}
)

target_link_libraries(dbtool abdb abscommon)

install(TARGETS dbtool
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
