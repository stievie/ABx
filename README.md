# ABx

Ancient Greece Multiplayer Online RPG.

## Architecture

Uses a single TCP stream for the game protocol.

## Run

~~~
abserv [-conf <config file>] [-log <log dir>]
~~~

## Third Party

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
