# Linux

1. Install CLang
2. Install premake5
    1 Clone repo https://github.com/premake/premake-core
    2. `cd` to premake-core
    3. `make -f Bootstrap.mak linux`. premake5 is now bin/release
    4. Add alias `premake5` to `~/premake-core/bin/release/premake5`
3. Install boost `sudo apt-get install libboost-all-dev`. Unfortunately the data server still needs some header files.
  (Set `BOOST_DIR` and `BOOST_LIB_PATH`)
4. Install `sudo apt-get install uuid-dev`
5. Install `libmysqlclient-dev`
6. Install `libpq-dev`
7. Install `libsqlite3-dev`
3. Clone repo `git -c http.sslVerify=false clone https://stievie.mooo.com/git/Trill/ABx.git`
4. Generate make files: GCC: `~/premake-core/bin/release/premake5 gmake`), CLang: `~/premake-core/bin/release/premake5 --cc=clang gmake`
5. cd ./build && make -f abdata.make

## Fixes

* Remove `#pragma comment` to link file. 
* Link `lua`, `abcrypto`, `libpq` (https://packages.debian.org/jessie/armhf/libpq-dev/filelist), `libmysqlclient` (https://packages.debian.org/jessie/armhf/libmysqlclient-dev/filelist), `libsqlite3` (https://packages.debian.org/jessie/armhf/libsqlite3-dev/filelist)
* Remove `#pragma once`, add include guards?
* `#ifdef _MSC_VER` all `*_s` C functions
