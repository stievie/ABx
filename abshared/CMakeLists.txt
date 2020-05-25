project (abshared CXX)

file(GLOB ABSHARED_SOURCES      abshared/*.cpp abshared/*.h)

add_library(
    abshared
    ${ABSHARED_SOURCES}
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(abshared PRIVATE -fno-exceptions -fno-rtti)
elseif(MSVC)
    target_compile_options(abshared PRIVATE /GR-)
endif()

target_precompile_headers(abshared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abshared/stdafx.h)

target_include_directories(abshared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
