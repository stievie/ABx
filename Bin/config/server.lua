--------------------------------------------------------------------------------
-- Game Server Settings --------------------------------------------------------
--------------------------------------------------------------------------------
-- server ip (the ip that server listens on)
game_ip = "0.0.0.0"
-- game server port
game_port = base_port          -- 0xABE
-- Game host must resolve to the above game_ip. If empty the client uses the same host as for login
game_host = ""     -- emtpy use same host as for login
--game_host = "stievie.mooo.com"

ai_server = true
ai_server_ip = "0.0.0.0"
ai_server_port = 12345

-- DOS prevention
max_packets_per_second = 60
