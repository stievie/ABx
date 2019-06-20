#!/bin/sh

# Compresses only binaries, no data
if [ -f "./Bin.zip" ]; then rm -f "./Bin.zip"; fi

7z a -tzip Bin.zip abdata abmsgs ablogin abserv absmngr abfile ablb absadmin keygen
