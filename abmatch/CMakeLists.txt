project (abmatch CXX)

file(GLOB ABMATCH_SOURCES      abmatch/*.cpp abmatch/*.h)

add_executable(
    abmatch
    ${ABMATCH_SOURCES}
)

target_precompile_headers(abmatch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abmatch/stdafx.h)
set_property(TARGET abmatch PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(abmatch abscommon EASTL)
if (UNIX)
    target_link_libraries(abmatch DeathHandler)
endif()

install(TARGETS abmatch
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
