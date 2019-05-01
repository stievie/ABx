# Linux

1. Install CLang
2. Install premake5
    1 Clone repo https://github.com/premake/premake-core
    2. `cd` to premake-core
    3. `make -f Bootstrap.mak linux`. premake5 is now bin/release
    4. Add alias `premake5` to `~/premake-core/bin/release/premake5`
3. Install boost `sudo apt-get install libboost-all-dev`. Unfortunately the data server still needs some header files. (Set `BOOST_DIR` and `BOOST_LIB_PATH`)
4. Install `sudo apt-get install uuid-dev` (https://packages.debian.org/jessie/uuid-dev)
5. Install `libmysqlclient-dev` (https://packages.debian.org/jessie/armhf/libmysqlclient-dev)
6. Install `libpq-dev` (https://packages.debian.org/jessie/armhf/libpq-dev)
7. Install `libsqlite3-dev` (https://packages.debian.org/jessie/armhf/libsqlite3-dev)
x. libldap2-dev, libssl-dev, libgsasl7-dev
8. Clone repo `git -c http.sslVerify=false clone https://stievie.mooo.com/git/Trill/ABx.git`.
9. `cd ABx`
10. Generate make files: GCC: `~/premake-core/bin/release/premake5 gmake`, CLang: `~/premake-core/bin/release/premake5 --cc=clang gmake`
11. `cd build`, maybe `make -f ... clean`, or just delete `obj` dir.
12. Build abdata: `make -f lua.make && make -f abcrypto.make && make -f abscommon.make && make -f abdata.make`

## Fixes

* Remove `#pragma comment` to link file. 
* Link `lua`, `abcrypto`, `libpq` (https://packages.debian.org/jessie/armhf/libpq-dev/filelist), 
`libmysqlclient` (https://packages.debian.org/jessie/armhf/libmysqlclient-dev/filelist), 
`libsqlite3` (https://packages.debian.org/jessie/armhf/libsqlite3-dev/filelist), 
`libuuid` (https://packages.debian.org/jessie/armhf/uuid-dev/filelist)
* Remove `#pragma once`, add include guards?
* `#ifdef _MSC_VER` all `*_s` C functions
* Fix initialization order
* Complie `make -f abdata.make -e config=release_armv7`

Disable link time optimiztation on linux

## Manjaro/Arch

* Install MariaDB, PostgreSQL, SQlite, Lua. Link against the installed libraries.
* Use Code::Blocks and build ./absall/absall.worklspace
* Use `cbp2mak` to create makefiles `cbp2make -in ./absall/absall.workspace -unix` and run `make -f absall.workspace.mak config=release`

### Adding Clang to `cbp2mak`s configuration:

In `/home/<user>/.cbp2make/cbp2make.cfg` add:

~~~xml
        <toolchain platform="Unix" alias="clang">
            <option generic_switch="-" />
            <option define_switch="-D" />
            <option include_dir_switch="-I" />
            <option library_dir_switch="-L" />
            <option link_library_switch="-l" />
            <tool type="compiler" alias="clang_c_compiler">
                <option description="Clang C Compiler" />
                <option program="clang" />
                <option make_variable="CC" />
                <option command_template="$compiler $options $includes -c $file -o $object" />
                <option source_extensions="c cc" />
                <option target_extension="o" />
                <option generic_switch="-" />
                <option need_quoted_path="0" />
                <option need_full_path="0" />
                <option need_unix_path="0" />
                <option include_dir_switch="-I" />
                <option define_switch="-D" />
            </tool>
            <tool type="compiler" alias="clang_cpp_compiler">
                <option description="Clang C++ Compiler" />
                <option program="clang++" />
                <option make_variable="CXX" />
                <option command_template="$compiler $options $includes -c $file -o $object" />
                <option source_extensions="cpp cxx" />
                <option target_extension="o" />
                <option generic_switch="-" />
                <option need_quoted_path="0" />
                <option need_full_path="0" />
                <option need_unix_path="0" />
                <option include_dir_switch="-I" />
                <option define_switch="-D" />
            </tool>
            <tool type="static_library_linker" alias="gnu_static_linker">
                <option description="GNU Static Library Linker" />
                <option program="ar" />
                <option make_variable="AR" />
                <option command_template="$lib_linker rcs $static_output $link_objects" />
                <option source_extensions="o obj" />
                <option target_extension="a" />
                <option generic_switch="-" />
                <option need_quoted_path="0" />
                <option need_full_path="0" />
                <option need_unix_path="0" />
                <option library_dir_switch="-L" />
                <option link_library_switch="-l" />
                <option object_extension="o" />
                <option library_prefix="lib" />
                <option library_extension="a" />
                <option need_library_prefix="0" />
                <option need_library_extension="0" />
                <option need_flat_objects="0" />
            </tool>
            <tool type="dynamic_library_linker" alias="gnu_dynamic_linker">
                <option description="Clang Dynamic Library Linker" />
                <option program="clang++" />
                <option make_variable="LD" />
                <option command_template="$linker -shared $libdirs $link_objects $link_resobjects -o $exe_output $link_options $libs" />
                <option source_extensions="o obj" />
                <option target_extension="a" />
                <option generic_switch="-" />
                <option need_quoted_path="0" />
                <option need_full_path="0" />
                <option need_unix_path="0" />
                <option library_dir_switch="-L" />
                <option link_library_switch="-l" />
                <option object_extension="o" />
                <option library_prefix="lib" />
                <option library_extension="dll" />
                <option need_library_prefix="0" />
                <option need_library_extension="0" />
                <option need_flat_objects="0" />
            </tool>
            <tool type="executable_binary_linker" alias="gnu_executable_linker">
                <option description="GNU Executable Binary Linker" />
                <option program="clang++" />
                <option make_variable="LD" />
                <option command_template="$linker $libdirs -o $exe_output $link_objects $link_resobjects $link_options $libs" />
                <option source_extensions="o obj" />
                <option target_extension="" />
                <option generic_switch="-" />
                <option need_quoted_path="0" />
                <option need_full_path="0" />
                <option need_unix_path="0" />
                <option library_dir_switch="-L" />
                <option link_library_switch="-l" />
                <option object_extension="o" />
                <option library_prefix="" />
                <option library_extension="" />
                <option need_library_prefix="0" />
                <option need_library_extension="0" />
                <option need_flat_objects="0" />
                <option option_wingui="-mwindows" />
            </tool>
~~~