# ABx

Ancient Greece Multiplayer Online RPG.

## Architecture

Uses a single TCP stream for the game protocol.

## Run Server

1. Run the data server `abdada`
2. Run the game server `abserv`

~~~
abserv [-conf <config file>] [-log <log dir>]
~~~

The Data server provides data from the database server and caches them. The Game
Server connects to the Data server.

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

## Run Client

1. Start `fw.exe`
2. Create an account
3. Create a character
4. Enter world

## Credits

### Server

* asio https://think-async.com/Asio/AsioStandalone
* Lua 5.3.4 https://www.lua.org/
* Kaguya https://github.com/satoren/kaguya
* pugixml https://pugixml.org/
* Recast & Detour https://github.com/recastnavigation/recastnavigation
* SQLite https://sqlite.org/
* GJK https://github.com/xuzebin/gjk

### Client

* Urho3D https://urho3d.github.io/
* asio https://think-async.com/Asio/AsioStandalone
* https://github.com/kainjow/Mustache
