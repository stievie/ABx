--------------------------------------------------------------------------------
-- Game Server Settings --------------------------------------------------------
--------------------------------------------------------------------------------
-- server ip (the ip that server listens on)
game_ip = "192.168.1.51"
-- game server port
game_port = base_port          -- 0xABE
-- Game host must resolve to the above game_ip. If empty the client uses the same host as for login
game_host = ""     -- emtpy use same host as for login
--game_host = "stievie.mooo.com"

-- DOS prevention
max_packets_per_second = 60
