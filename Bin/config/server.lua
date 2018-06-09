--------------------------------------------------------------------------------
-- Server Settings -------------------------------------------------------------
--------------------------------------------------------------------------------
-- server ip (the ip that server listens on)
ip = "192.168.1.51"

-- game server port
-- game server must be on it's own port (due to limits of the protocol)
game_port = base_port          -- 0xABE
-- Game host must resolve to the above ip. If empty the client uses the same host as for login
--game_host = "stievie.mooo.com"
-- game_ip = ip

-- DOS prevention
max_packets_per_second = 60
