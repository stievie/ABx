CC = gcc
CXX = g++
AR = ar

CXXFLAGS = -MMD -O3 -Wall -Wextra -std=c++14
LINKCMD = $(AR) -rcs "$@"
MKDIR_P = mkdir -p
