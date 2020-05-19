cmake_minimum_required (VERSION 3.17)
project (abdb CXX)

file(GLOB_RECURSE ABDB_SOURCES      abdb/*.cpp abdb/*.h)

add_library(
    abdb
    ${ABDB_SOURCES}
)

if (ABX_PGSQL)
  if (WIN32)
    include_directories(../Include/pgsql)
  endif()
  target_compile_definitions(abdb PRIVATE USE_PGSQL)
elseif (ABX_MYSQL)
  if (WIN32)
    include_directories(../Include/mysql)
  endif()
  target_compile_definitions(abdb PRIVATE USE_MYSQL)
elseif (ABX_ODBC)
  target_compile_definitions(abdb PRIVATE USE_ODBC)
elseif (ABX_SQLITE)
  target_compile_definitions(abdb PRIVATE USE_SQLITE)
endif()

target_link_libraries(abdb abscommon)

target_include_directories(abdb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
