# Windows

Visual Studio 2017 required. Open `.sln` files and build.

Didn't try with Visual Studio 2019 tools chain yet.

# Linux

Install GCC, ccache.

It should also compile with Clang out of the box, just change `CXX` to `clang++` and `CC` to `clang`
in `makefiles/makefile.common`. But we keep GCC for various reasons.

## Dependencies

* `uuid-dev` (https://packages.debian.org/jessie/uuid-dev)
* `libmysqlclient-dev` (https://packages.debian.org/jessie/armhf/libmysqlclient-dev) or MariaDB
* `libpq-dev` (https://packages.debian.org/jessie/armhf/libpq-dev)
* `libsqlite3-dev` (https://packages.debian.org/jessie/armhf/libsqlite3-dev)
* libldap2-dev, libssl-dev, libgsasl7-dev, libkrb5-dev, lua5.3, lua5.3-dev

## Build

1. Install the required packages.
2. `cd` to `./makefiles`
3. Type `make`

## Trouble shooting

1. Link error multiple definition of: Clean -> Build, delete all `*.a` files. `make clean`

## Client

### Dependencies

Required lib files for the client not in this repostitory:

* `Urho3D(_d).lib`

Build them and put the library files in `./Lib/x64`.

### Build

`cd` to `./makefiles` and type `make -f client.make`.

