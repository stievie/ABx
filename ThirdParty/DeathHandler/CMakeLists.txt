project (DeathHandler CXX)

file(GLOB DEATHHANDLER_SOURCES      ./death_handler.cc ./death_handler.h)

add_library(
    DeathHandler
    ${DEATHHANDLER_SOURCES}
)

if (UNIX)
    target_link_libraries(DeathHandler dl)
endif()
