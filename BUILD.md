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

## Linux

### Prerequisites

* GCC 9 or later
* GNU make
* ccache (optional)

### Dependencies

* `uuid-dev`
* `libmysqlclient-dev` or MariaDB when building with `USE_MYSQL`
* `libpq-dev` when building with `USE_PGSQL`
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

### Build

1. Install the required packages.
2. `cd` to `./makefiles`
3. Type `make`.

It can use ccache to speedup compilation. Export the `PRE_CXX` environment variable.
~~~sh
export PRE_CXX=ccache
~~~

On Linux you could also use CMake. If you are in the root directory:
~~~sh
mkdir build
cd build
cmake ..
make
~~~
Of if you prefer Ninja:
~~~sh
mkdir build
cd build
cmake -G Ninja ..
ninja
~~~

### Client

#### Dependencies

Required lib files for the client not in this repository:

* `Urho3D(_d).lib`

<s>Clone Urho3D from the official GitHub repository (https://github.com/urho3d/Urho3D, use the latest master).</s>
Unfortunately the Client does not compile with the current Urho3D master branch.

Clone my fork of Urho3D from https://github.com/stievie/Urho3D and switch to the `abx` branch:
~~~sh
$ git clone https://github.com/stievie/Urho3D
$ cd Urho3D
$ git checkout -b abx
~~~

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

`cd` to `./makefiles` and type:
~~~sh
$ make -f client.make
~~~

## Troubleshooting

If you have built it before and it doesn't build anymore, try to clean it before:

~~~sh
$ make clean && make
~~~
