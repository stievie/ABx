# ABx

Ancient Greece Multiplayer Online RPG.

* https://www.gamedev.net/blogs/blog/3042-abx-online-rpg/
* https://dev.azure.com/trill42/ABx

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

There can be only *one* login server, since all clients connect to this server.
Once the client is authenticated, the connection to the login server is closed.

### Game Server

The game server simulates the games. There can be any number of game servers, but
if they run on the same machine, the game servers must listen on different ports.

Since all game server connect to the same data server, all game server share the
same data. Because this game is designed to have an instanced world, several game
server instances appear to the player as one game server. It is even possible to
change the game server with a simple menu click.

So, in theory, how many players and games this server can handle, depends only 
on how much hardware you have.

### Message Server

This one is for inter server communication, for example to deliver messages from
a player to another player on a different game server.

### Load Balancer (`ablb`)

An optional load balancer/proxy to make it possible to have several Login Server.
It'll lookup running Login Servers and bridge client connections to one of them.

### Admin Server

A Web interface.

### Client

Uses a single TCP stream for the game protocol.

## Run Server

1. Run the data server `abdada`
2. Run the message server `abmsgs`
3. Run the file server `abfile`
4. Run the login server `ablogin`
5. Run the game server `abserv`

Optional run `absadmin`.

### Run as NT service

Define `WIN_SERVICE` and recompile the server. It will create a Windows Service
application. Use the `*_svc.lua` configuration files instead to log to a file instead of
StdOut.

Install the services with the `-install` command line switch. Use `-remove` to
uninstall it.

### MySQL

You may want to increase `max_allowed_packet` in `my.cnf` / `my.ini`  in the `[mysqld]`
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

### PostgreSQL

Doesn't need any special attention. Works with PostreSQL 9 to 11.

### Structure of `data` directory

~~~
  data
   - maps
     - (Map)
       - index.xml
       - NavMesh.bin
       - Scene.xml
       - HeightField.terrain
   - models
   - scripts
     - actors
       - logic
       - npcs
     - ai
     - effects
       - conditions
       - ...
     - games
     - items (armor, mods etc.)
     - skills
~~~

## Run Client

1. Start `fw.exe`
2. Create an account
3. Create a character
4. Enter world

## Encoding

Everything is UTF-8 encoded. The Client sends only UTF-8 encoded strings to the
server. All files on the server are UTF-8 encoded. So there is no additional 
conversion required.

## Build

Use premake5 to generate solutions for the server and the client.

1. Server: Open `build/abs3rd.sln` and `build/absall.sln` and compile.
2. Client: Open `build/abclient.sln` and compile.

## Credits

See `CREDITS.md`
