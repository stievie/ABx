#!/bin/sh

valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes $1

