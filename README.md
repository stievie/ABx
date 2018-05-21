# ABx

Ancient Greece Multiplayer Online RPG.

## Architecture

Uses a single TCP stream for the game protocol.

## Run Server

1. Run the data server `abdada`
1. Run the file server `abfile`
2. Run the game server `abserv`

~~~
abserv [-conf <config file>] [-log <log dir>]
~~~

The Data server provides data from the database server and caches them. The Game
Server connects to the Data server.

The file server is a simple HTTP server providing files and other information.
The client my connect to it from time to time and download data.

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

~~~
[mysqld]
# ...
max_allowed_packet = 32M
~~~

Also adding `skip-name-resolve` to `[mysqld]` may be a good idea.

~~~
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

## Credits

See `CREDITS.md`
