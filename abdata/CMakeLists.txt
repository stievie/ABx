cmake_minimum_required (VERSION 3.17)
project (abdata CXX)

file(GLOB_RECURSE ABDATA_SOURCES      abdata/*.cpp abdata/*.h)

add_executable(
    abdata
    ${ABDATA_SOURCES}
)

if (ABX_PGSQL)
  target_compile_definitions(abdata PRIVATE USE_PGSQL)
elseif (ABX_MYSQL)
  target_compile_definitions(abdata PRIVATE USE_MYSQL)
elseif (ABX_ODBC)
  target_compile_definitions(abdata PRIVATE USE_ODBC)
elseif (ABX_SQLITE)
  target_compile_definitions(abdata PRIVATE USE_SQLITE)
endif()

target_link_libraries(abdata abdb abscommon EASTL)
