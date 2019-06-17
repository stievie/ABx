# Tools chain
CC = clang
CXX = clang++
AR = ar
# Utils
MKDIR_P = mkdir -p
# Linker Static libs
LINKCMD_LIB = $(AR) -r "$@"
# Linker Executeables
LINKCMD_EXE = $(CXX) $(LDFLAGS) -o "$@"

# Flags
CFLAGS = -MMD -m64 -O3 -Wall -Wextra
CXXFLAGS = -MMD -m64 -O3 -Wall -Wextra -std=c++14
LDFLAGS =  -L../Lib -L../Lib/x64/Release -m64 -s

# Options
DEFINES = -DNDEBUG -D_NPROFILING
INCLUDES = -I../Include
