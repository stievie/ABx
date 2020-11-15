project (fhash CXX)

file(GLOB SOURCES
    fhash/*.cpp
    fhash/*.h
)

add_executable(
    fhash
    ${SOURCES}
)

target_link_libraries(fhash absync)

install(TARGETS fhash
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
