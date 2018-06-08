# TODO

* abserv, abmsgs: Notify new mail not reliable?
* Mail not marked as read

* abserv: Move Map::LoadScene() to new static class IOMap::Load(Game::Map& map, const std::string& name).

* abserv: Remove status, admin (?) protocol
* abserv: Add gamehost, gameport command line switches
* abadmin: Admin HTTP server. Browser interface to admin all servers

* ablogin, abserv: Crash on exit with Ctrl+C (maybe fixed)

* abdata: Clean expired guild members

* abserv: Remove Status protocol, add info to AB::Entities::Service
* abserv: Admin protocol

* all: /friend /ignore /flremove /fl commands
* abserv: Lua API (Sphere) Octree query
* abdata: status, admin virtual entity to query status do admin stuff
* all: Item Database
  https://github.com/jgoodman/MySQL-RPG-Schema   
  https://www.gamedev.net/forums/topic/465300-rpg-item-database/
* Admin/monitoring GUI/CLI interface for abserv, abdata, abfile
  GUI https://github.com/kosenko/ui (https://kosenko.github.io/boost.ui/), 
    https://github.com/cnjinhao/nana, 
    https://github.com/andlabs/libui,
    https://github.com/ocornut/imgui
