--------------------------------------------------------------------------------
-- Server Settings -------------------------------------------------------------
--------------------------------------------------------------------------------
-- server ip (the ip that server listens on)
ip = "192.168.1.51"

-- login server port
-- This should be the port used for connecting with IP changers etc.
login_port = 2748         -- 0xABC
-- login_ip = ip

-- game server port
-- game server must be on it's own port (due to limits of the protocol)
game_port = 2749          -- 0xABE
-- Game host must resolve to the above ip. If empty the client uses the same host as for login
--game_host = "stievie.mooo.com"
-- game_ip = ip

-- status port
-- Used by status protocol connections, should be same as login
-- to work correctly with server lists etc.
status_port = login_port
-- status_ip = login_ip

-- admin port
-- Port used by the Admin protocol
admin_port = 2750         -- 0xABF
-- admin_ip = ip

-- The file server sent to the client. If host is empty the client uses the login host.
--file_host = "stievie.mooo.com"
file_port = 8081

status_timeout = 30 * 1000

-- DOS prevention
max_packets_per_second = 60
