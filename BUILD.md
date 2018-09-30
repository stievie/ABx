# Linux

0. Install CLang
1. Install premake5
  1.1 Clone repo https://github.com/premake/premake-core
  1.2. `cd` to premake-core
  1.3. `make -f Bootstrap.mak linux`. premake5 is now bin/release  
2. Install boost `sudo apt-get install libboost-all-dev`. Unfortunately the data server still needs some header files.
  (Set `BOOST_DIR` and `BOOST_LIB_PATH`)
3. Clone repo `git -c http.sslVerify=false clone https://stievie.mooo.com/git/Trill/ABx.git`
4. `~/premake-core/bin/release/premake5 gmake` `~/premake-core/bin/release/premake5 --cc=clang gmake`
