project (abupdate CXX)

file(GLOB SOURCES
    abupdate/*.cpp
    abupdate/*.h
)

add_executable(
    abupdate
    ${SOURCES}
)

target_link_libraries(abupdate absync libabclient)

install(TARGETS abupdate
    RUNTIME DESTINATION abclient/bin
    COMPONENT runtime
)
