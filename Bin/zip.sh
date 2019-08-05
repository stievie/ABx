#!/bin/sh

if [ -f "./abserv.zip" ]; then rm -f "./abserv.zip"; fi

7z a -tzip abserv.zip abdata abmsgs abmatch ablogin abserv abfile ablb absadmin keygen server.crt server.csr server.key abserver.dh 00run.sh *.lua config\* data\* file_root\* admin\README.md admin\templates\* admin\root\*.ico admin\root\css\* admin\root\fonts\* admin\root\images\* admin\root\js\* admin\root\files\.gitkeep logs\*\.gitkeep recordings\.gitkeep ..\README.md

# Not added to the ZIP archive:
# - admin\root\vendors
