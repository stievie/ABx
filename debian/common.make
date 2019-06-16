# Tools chain
CC = gcc
CXX = g++
AR = ar
# Utils
MKDIR_P = mkdir -p
# Linker Static libs
LINKCMD_LIB = $(AR) -rcs "$@"
# Linker Executeables
LINKCMD_EXE = $(CXX) $(LDFLAGS) -o "$@"

# Flags
CFLAGS = -MMD -m64 -O3 -Wall -Wextra
CXXFLAGS = -MMD -m64 -O3 -Wall -Wextra -std=c++14
LDFLAGS =  -L../Lib -L../Lib/x64/Release -m64

# Options
DEFINES = -DNDEBUG -D_NPROFILING
INCLUDES = -I../Include
