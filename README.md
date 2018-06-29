# ABx

Ancient Greece Multiplayer Online RPG.

## Architecture

~~~
                                              +--------------------- File System
         +---------------- File Server1...N --+-----+-- Data Server -+
         +-----------------+                  |     |                +---- Database
Client1 -+                 +--- Game Server1 -+--+  |
  ...    + - Login Server -+        ...       |  +--+-- Message Server
ClientN -+                 +--- Game ServerN -+--+
         +-----------------+
~~~

### Data Server

The Data server provides data from the database server and caches it. It also
acts as threadsafe interserver shared memory. Connection to this server is 
required for:

* File Server
* Login Server
* Game Server

There can be only *one* data server. It is the central point to which all other
servers connect, and get their data from.

### File Server

The file server is a simple HTTP server providing files and other information.
The client may connect to it from time to time and download data.

There can be any number of file servers. Usually you may want to have file servers
in different regions, and not on the same machine. A file server does not need
much resources, just bandwidth. But if they run on the same machine, they must
listen on different ports.

### Login Server

Used by the client to login, create accounts and manage characters.

It also tells the client to which Game and File Server to connect.

There can be only *one* login server, since all clients connects to this server.
Once the client is authenticated, the connection to the login server is closed.

### Game Server

The game server simulates the games. There can be any number of game servers, but
if they run on the same machine, the game servers must listen on different ports.

Since all game server connect to the same data server, all game server share the
same data.

So, in theory, how many players and games this server can handle, depends only 
on how much hardware you have.

### Message Server

This one is for inter server communication, for example to deliver messages from
a player to another player on a different game server.

### Client

Uses a single TCP stream for the game protocol.

## Run Server

1. Run the data server `abdada`
2. Run the file server `abfile`
3. Run the login server `ablogin`
4. Run the game server `abserv`

~~~
abserv [-conf <config file>] [-log <log dir>]
~~~

### Run as NT service

Use NSSM: The Non-Sucking Service Manager. Install `abdata` as service with automatic 
start. Install `abserv` as service with *Delayed* automatic start. `abserv` must start
after `abdata`.

`abdata` Arguments
: `-conf "<Path To>/Bin/abdata_svc.lua"`

`abserv` Arguments
: `-conf "<Path To>/Bin/config_svc.lua" -log "<Path To>/Bin/logs/abserv"`

### MySQL

You may want to increase `max_allowed_packet` in `my.cnf / my.ini`  in the `[mysqld]`
section to e.g. `32M`, if the data server loses connection from time to time.

~~~ini
[mysqld]
# ...
max_allowed_packet = 32M
~~~

Also adding `skip-name-resolve` to `[mysqld]` may be a good idea.

~~~ini
[mysqld]
# ...
skip-name-resolve
~~~

### Structure of `data` directory

~~~
  data
   - scripts
     - creatures
     - effects
       - conditions
       - ...
     - games
     - items (armor, mods etc.)
     - skills
   - maps (navmesh, terrain etc.)
~~~

## Run Client

1. Start `fw.exe`
2. Create an account
3. Create a character
4. Enter world

## Build

Use premake5 to generate solutions for the server and the client.

1. Server: Open `build/absall.sln` and compile.
2. Client: Open `build/abclient.sln` and compile.

## Credits

See `CREDITS.md`
