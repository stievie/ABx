project (abdb CXX)

file(GLOB ABDB_SOURCES      abdb/*.cpp abdb/*.h)

add_library(
    abdb
    ${ABDB_SOURCES}
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(abdb PRIVATE -fno-exceptions -fno-rtti)
elseif(MSVC)
    target_compile_options(abdb PRIVATE /GR-)
endif()

if (ABX_PGSQL)
    # On Ubuntu also postgresql-server-dev-all must be installed
    include(FindPostgreSQL)
    if (PostgreSQL_FOUND)
        target_include_directories(abdb PRIVATE ${PostgreSQL_INCLUDE_DIRS})
        target_link_libraries(abdb ${PostgreSQL_LIBRARIES})
    else()
        message(WARNING "No PostgreSQL installation found")
        target_include_directories(abdb PRIVATE ${CMAKE_SOURCE_DIR}/Include/pgsql)
        if (WIN32)
            target_link_libraries(abdb pq)
        else()
            target_link_libraries(abdb pq ldap ssl crypto z gssapi_krb5 dl)
        endif()
    endif()

    target_compile_definitions(abdb PUBLIC USE_PGSQL)
elseif (ABX_MYSQL)
    if (WIN32)
        target_include_directories(abdb PRIVATE ../Include/mysql)
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
