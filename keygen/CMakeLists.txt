project (keygen CXX)

file(GLOB KEYGEN_SOURCES      keygen/*.cpp keygen/*.h)

add_executable(
    keygen
    ${KEYGEN_SOURCES}
)

target_precompile_headers(keygen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/keygen/stdafx.h)

target_link_libraries(keygen abscommon)

install(TARGETS keygen
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
