# Build

## Supported Platforms/Tools

The server needs a C++17 compiler.

The Server is  known to compile on the following platforms with the following
tools chains:

* Windows 7 with VS 2019
* Debian, Ubuntu, Arch, Manjaro Linux with GCC 9 and newer.

## Windows

Visual Studio 2019 required. Open `.sln` files and build. There are `.sln` files in `absall` and
in the directories of the projects.

### Server

Build `absall\abs3rd.sln` and `absall\absall.sln`.

### Client

Build `absall\abs3rd.sln` and `abclient\abclient.sln`.

The client links D3D11 libraries, this means you should build also Urho3D for D3D11. In CMake check
`URHO3D_D3D11`. This makes it using the D3D11 graphics API instead of D3D9.

![Urho3D CMake](/Doc/urho3d_cmake.png?raw=true)

If you build Urho3D with the default settings (it uses D3D9 then), you must add
`d3d9.lib` as linker input when you encounter some unresolved symbol linker error,
saying `error LNK2001: unresolved external symbol Direct3D(Something)`.

To enable auto updating, compile the client with `AUTOUPDATE_ENABLED` defined,
CMake option `ABX_CLIENT_AUTOUPDATE`, see [Autoupdate](/INSTALL.md#autoupdate).

## Linux

### Prerequisites

* GCC 9 or later
* GNU make
* CMake 3.16
* ccache (optional)

### Dependencies

* `uuid-dev`
* `libmysqlclient-dev` or MariaDB when building with `USE_MYSQL`
* `libpq-dev` when building with `USE_PGSQL`, (`postgresql-server-dev-all` when using CMake on Ubuntu)
* `libsqlite3-dev` when building with `USE_SQLITE`
* `libssl-dev`
* `libldap2-dev` for PostgreSQL, when building with `USE_PGSQL`
* `libgsasl7-dev` for PostreSQL, when building with `USE_PGSQL`
* `libkrb5-dev` for PostgreSQL, when building with `USE_PGSQL`
* `lua5.3`
* `lua5.3-dev`
* `libncurses-dev` for the Debug client (`dbgclient`).

Your distribution may have different names for these libraries.

I didn't try any other database backend than PostgreSQL since some time, so the others may or may
not work.

### Install GCC and Dependencies on Ubuntu

The following works for Ubuntu 18.04:

~~~sh
# Install GCC 9
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update -qq
sudo apt-get install g++-9
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
# Install Dependencies
sudo -E apt-get -yq --no-install-suggests --no-install-recommends install uuid-dev libpq-dev libssl-dev libldap2-dev libgsasl7-dev libkrb5-dev lua5.3 lua5.3-dev libncurses-dev
~~~

If you get a link error, like "Library -lz not found", you also need to install `zlib1g-dev`.

If CMake complains about missing PostgreSQL although you installed `libpq-dev`, also install
`postgresql-server-dev-all`, see https://stackoverflow.com/questions/13920383/findpostgresql-cmake-wont-work-on-ubuntu#40027295.


### Build

1. Install the required packages.
2. Use CMake to build it (assuming you want to use GNU make):
~~~sh
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
# Copy executables to ../bin and ../abclient/bin
$ cmake --install . --component runtime
~~~

After that, you can update easily with just (1) pulling the latest HEAD from GitHub, (2) cd into the previously created `build` directory and (3) type `make`.
If files were added or removed, you may need to re run CMake.

### Client

#### Dependencies

Required lib files for the client not in this repository:

* `Urho3D(_d).lib`

Clone Urho3D from the official GitHub repository (https://github.com/urho3d/Urho3D, use the latest master).

Build Urho3D and put the library files in `./Lib` (or `./Lib/x64/Debug`,
`/Lib/x64/Release`). Copy or symlink the directory with Urho3D header files to `Include/`,
so there is a `Include/Urho3D` directory, e.g:
~~~sh
$ ln -s /home/sa/src/Urho3D/include/Urho3D/ /home/sa/src/ABx/Include/
~~~

Copy Urho3D assets to the `bin` directory:
~~~plain
bin
  - Autoload
  - CoreData
  - Data
~~~

#### Build

Since everything has the same top level CMakeLists.txt file, the client is built with the server. But you must configure it to build the client. To change the above commands to build the client too:
~~~sh
$ mkdir build
$ cd build
$ cmake -DABX_BUILD_CLIENT=ON -DCMAKE_BUILD_TYPE=Release ..
$ make
# Copy executables to ../bin and ../abclient/bin
$ cmake --install . --component runtime
~~~
