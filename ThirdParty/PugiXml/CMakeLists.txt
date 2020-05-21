project (PugiXml CXX)

file(GLOB PUGIXML_SOURCES      src/*.cpp src/*.hpp)

add_library(
    PugiXml
    ${PUGIXML_SOURCES}
)
