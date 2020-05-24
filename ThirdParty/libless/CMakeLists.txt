project (less CXX)

file(GLOB LESS_SOURCES
    src/*.cpp
    src/css/*.cpp
    src/less/*.cpp
    src/lessstylesheet/*.cpp
    src/stylesheet/*.cpp
    src/value/*.cpp
    include/less/*.h
    include/less/css/*.h
    include/less/less/*.h
    include/less/lessstylesheet/*.h
    include/less/stylesheet/*.h
    include/less/value/*.h)

add_library(
    less
    ${LESS_SOURCES}
)

if (MSVC)
    target_compile_options(less PRIVATE /wd4458 /wd4702 /wd4706)
endif()

target_include_directories(less PUBLIC include/less)
