project (abmsgs CXX)

file(GLOB ABSMGS_SOURCES      abmsgs/*.cpp abmsgs/*.h)

add_executable(
    abmsgs
    ${ABSMGS_SOURCES}
)

target_precompile_headers(abmsgs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abmsgs/stdafx.h)
set_property(TARGET abmsgs PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(abmsgs abscommon EASTL)
if (UNIX)
    target_link_libraries(abmsgs DeathHandler)
endif()

install(TARGETS abmsgs
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
