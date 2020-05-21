project (abdb CXX)

file(GLOB ABDB_SOURCES      abdb/*.cpp abdb/*.h)

add_library(
    abdb
    ${ABDB_SOURCES}
)

if (ABX_PGSQL)
  if (WIN32)
    include_directories(../Include/pgsql)
    target_link_libraries(abdb pq)
  else()
    target_link_libraries(abdb pq ldap ssl crypto z gssapi_krb5 dl)
  endif()
  target_compile_definitions(abdb PUBLIC USE_PGSQL)
elseif (ABX_MYSQL)
  if (WIN32)
    include_directories(../Include/mysql)
  endif()
  target_compile_definitions(abdb PUBLIC USE_MYSQL)
elseif (ABX_ODBC)
  target_compile_definitions(abdb PUBLIC USE_ODBC)
elseif (ABX_SQLITE)
  target_compile_definitions(abdb PUBLIC USE_SQLITE)
endif()

target_precompile_headers(abdb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abdb/stdafx.h)

target_link_libraries(abdb abscommon)

target_include_directories(abdb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
