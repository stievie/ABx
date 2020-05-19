cmake_minimum_required (VERSION 3.17)
project (abdata CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

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
if (UNIX)
	target_link_libraries(abdata pthread DeathHandler dl uuid)
endif()
