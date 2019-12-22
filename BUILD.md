# Build

## Supported Platforms/Tools

The server needs a C++17 compiler.

The Server is  known to compile on the following platforms with the following
tools chains:

* Windows 7 with VS 2017 (don't have Windows 10)
* Debian, Ubuntu, Arch, Manjaro Linux with GCC 8 and newer.

## Windows

Visual Studio 2017 required. Open `.sln` files and build. There are `.sln` files in `absall` and
in the directories of the projects.

I use Visual Studio 2019 as IDE with the Visual Studio 2017 tools chain. I didn't try the Visual
Studio 2019 tools chain yet.

## Linux

### Prerequisites

* GCC
* GNU make
* ccache

It should also compile with Clang out of the box, just change `CXX` to `clang++` and `CC` to `clang`
in `makefiles/makefile.common`. But we keep GCC for various reasons.

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

Your distribution may have different names for these libraries.

I didn't try any other database backend than PostgreSQL since some time, so the others may or may
not work.

### Build

1. Install the required packages.
2. `cd` to `./makefiles`
3. Type `make`.

### Client

#### Dependencies

Required lib files for the client not in this repository:

* `Urho3D(_d).lib`

Clone Urho3D from GitHub (https://github.com/urho3d/Urho3D, I think version 1.7
should work), then apply the patch `Urho3D.patch` to it. If you don't want to do
it, this patch just adds an inline function to the header `UI/CheckBox.h`, you
can do it manually:
~~~cpp
    /// Set checked state without generating an event.
    void SetCheckedInternal(bool enable){ checked_ = enable; }
~~~
Build Urho3D and put the library files in `./Lib/x64`. Copy Urho3D header files to
`Include/Urho3D`. Copy Urho3D assets to the `bin` directory.

#### Build

`cd` to `./makefiles` and type `make -f client.make`.

