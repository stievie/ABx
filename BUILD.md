# Windows

Visual Studio 2017 required. Open `.sln` files and build. There are `.sln` files in `absall` and
in the directories of the projects.

I use Visual Studio 2019 as IDE with the Visual Studio 2017 tools chain. I didn't try the Visual 
Studio 2019 tools chain yet.

# Linux

## Prerequisites

* GCC
* GNU make
* ccache

It should also compile with Clang out of the box, just change `CXX` to `clang++` and `CC` to `clang`
in `makefiles/makefile.common`. But we keep GCC for various reasons.

## Dependencies

* `uuid-dev`
* `libmysqlclient-dev` or MariaDB when building with `USE_MYSQL`
* `libpq-dev` when building with `USE_PGSQL`
* `libsqlite3-dev` when building with `USE_SQLITE`
* libssl-dev
* libldap2-dev (for PostgreSQL)
* libgsasl7-dev (for PostreSQL)
* libkrb5-dev (for PostgreSQL)
* lua5.3
* lua5.3-dev

Your distribution may have different names for these libraries.

I didn't try any other database backend than PostgreSQL since some time, so the others may or may
not work.

## Build

1. Install the required packages.
2. `cd` to `./makefiles`
3. Type `make`.

## Client

### Dependencies

Required lib files for the client not in this repostitory:

* `Urho3D(_d).lib`

Build them and put the library files in `./Lib/x64`.

### Build

`cd` to `./makefiles` and type `make -f client.make`.

