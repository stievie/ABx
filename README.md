# ABx

[![Build Status](https://dev.azure.com/trill42/ABx/_apis/build/status/stievie.ABx?branchName=master)](https://dev.azure.com/trill42/ABx/_build/latest?definitionId=3&branchName=master)

Ancient Greece Multiplayer Online RPG.

https://devtube.dev-wiki.de/video-channels/trill_channel/videos

https://www.gamedev.net/projects/1587-abx

## License

The source code is licensed under the MIT License.

Assets, Data, Art made by me is licensed under the [CC BY 2.0](https://creativecommons.org/licenses/by/2.0/).

## ETA

Christmas 2117 +/- some years. That's still a negative stardate so I think we are fine.

## The Goals

* Instanced world
* Primary and secondary professions (classes)
* Big skill pool
* Diverse skill builds and team builds
* Focus on tactics rather than kill everything that moves
* Cooperative and competitive game formats
* PvP and PvE

## Current features

* Uses a single TCP stream for the game protocol. This is not a shooter or action game, more like a classical online RPG.
* Database back-end is currently PostgreSQL.
* Instanced world.
* Encrypted Game and Login Protocol.
* Lag compensation: Entity interpolation, client prediction.
* Spawn any number of server which may have heavy load (game, file server) even on different hardware.
* Local (map), Guild, Team, Trade chat and whisper is currently working, even across different game server.
* Friend/ignore list
* Persistent Mail
* Static objects are directly loaded from Urho3D's scene files.
* Lua API to script Games, NPCs, Skills, Effects, Items etc.
* Navigation using Recast/Detour
* Server runs on Windows and Linux
* Client runs on Windows and Linux
* Game AI, Behavior Tree
* [Mumble](https://www.mumble.info/) voice chat integration.

## Screenshot

![Screenshot](/Doc/screenshot.jpg?raw=true)

## Motivation

The only Computer Game I play is [Guild Wars](https://www.guildwars.com/) and I play it for
well over 12 years now. With my main account I played for over 12,000 hours, and I still
enjoy it. But the game is old, and does not get much love anymore. So maybe
it's time to create something like it.

However, the main motivation for me is, I think Guild Wars is an awesome piece of technology,
and I want to know if I can create something like it. I know the Art work is beyond
my Art skills (they are literally not existent), but with enough time it should be possible
to do the programming part.

There are some other nice side effects. It's exciting times to program in C++,
it is moving fast, but does not break things.

## Build, install, run

See [BUILD.md](BUILD.md) and [INSTALL.md](INSTALL.md).

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for more
informations.

## Credits

See [CREDITS.md](CREDITS.md).

## Architecture

![Architecture](/Doc/abschema.png?raw=true)

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

On Linux this server must be run as root, because it binds the ports 80 and 443.

### Client

Uses a single TCP stream for the game protocol.

## Run Server

1. Run the data server `abdada`
2. Run the message server `abmsgs`
3. Run the file server `abfile`
4. Run the login server `ablogin`
5. Run the game server `abserv`

Optional run `absadmin`.

All servers must run with the same timezone, preferably UTC.

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

### Structure of `Bin/data` directory

~~~plain
  data
   - maps
     - (Map)
       - index.xml  (defines the files)[0]
       - NavMesh.bin (Recast navigation mesh)
       - Scene.xml (Urho3D's scene file)
       - HeightField.terrain (Heightfield file)
   - models
   - scripts
     - actors
       - aoe (area of effect)
       - logic (portals etc)
       - npcs
       - ...
     - behaviors (behavior trees)
     - effects
       - conditions
       - ...
     - games
     - items (armor, weapons etc.)
     - includes (shared includes)
     - skills
     - quests
     - skills
     main.lua
~~~

[0] Map index.xml file format

~~~xml
<?xml version="1.0"?>
<index>
  <file type="Scene" src="Athena.xml" />
  <file type="NavMesh" src="all_tiles_navmesh.bin" />
  <file type="Terrain" src="athena.hm" />
</index>
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
