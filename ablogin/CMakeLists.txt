project (ablogin CXX)

file(GLOB ABLOGIN_SOURCES      ablogin/*.cpp ablogin/*.h)

add_executable(
    ablogin
    ${ABLOGIN_SOURCES}
)

target_precompile_headers(ablogin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ablogin/stdafx.h)

# We need some header files from abshared
target_link_libraries(ablogin abscommon abshared)
if (UNIX)
    target_link_libraries(ablogin DeathHandler)
endif()

install(TARGETS ablogin
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
