# Tools chain
CC = gcc
CXX = g++
AR = ar
# Utils
MKDIR_P = mkdir -p

# Flags
CFLAGS = -MMD -m64 -O3 -Wall -Wextra
CXXFLAGS = -MMD -m64 -O3 -Wall -Wextra -std=c++14
LDFLAGS =  -L../Lib -L../Lib/x64/Release -m64
# Static libs
LINKCMD_LIB = $(AR) -rcs "$@"
# Executeables
LINKCMD_EXE = $(CXX) $(LDFLAGS) -o "$@"
