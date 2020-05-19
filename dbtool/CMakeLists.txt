cmake_minimum_required (VERSION 3.17)
project (dbtool CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE DBTOOL_SOURCES      dbtool/*.cpp dbtool/*.h)

add_executable(
    dbtool
    ${DBTOOL_SOURCES}
)

if (ABX_PGSQL)
  target_compile_definitions(dbtool PRIVATE USE_PGSQL)
elseif (ABX_MYSQL)
  target_compile_definitions(dbtool PRIVATE USE_MYSQL)
elseif (ABX_ODBC)
  target_compile_definitions(dbtool PRIVATE USE_ODBC)
elseif (ABX_SQLITE)
  target_compile_definitions(dbtool PRIVATE USE_SQLITE)
endif()

target_link_libraries(dbtool abdb abscommon)
