cmake_minimum_required (VERSION 3.17)
project (PugiXml CXX)

file(GLOB_RECURSE PUGIXML_SOURCES      src/*.cpp src/*.hpp)

add_library(
    PugiXml
    ${PUGIXML_SOURCES}
)
