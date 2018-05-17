
* abdata: Create should not be blocking. New method Create/CreateSynch? Or synch flag for all methods?
* abclient, abserv: LoginProtocol should use Account UUID instead of name except for first login.
* Check https://github.com/im95able/Rea for FL?
* all: /friend /ignore /flremove /fl commands
* abserv: Lua API (Sphere) Octree query
* all: Move all .lua scripts to /data/scripts/skills|effects|games|creatures|items
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
* all: Item Database
  https://github.com/jgoodman/MySQL-RPG-Schema   
  https://www.gamedev.net/forums/topic/465300-rpg-item-database/
* abfile: HTTP file server
  https://github.com/eidheim/Simple-Web-Server
