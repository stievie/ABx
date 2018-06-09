--------------------------------------------------------------------------------
-- Login Server Settings -------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "ef5ac3ec-45f9-40dc-8523-99c7937c69ba"
location = "AT"
server_name = "ablogin"

-- login server port
-- This should be the port used for connecting with IP changers etc.
login_port = 2748         -- 0xABC
login_ip = "192.168.1.51"
--login_host = "stievie.mooo.com"

-- DOS prevention
max_packets_per_second = 60

-- Data server
data_host = "localhost"
data_port = 2770

require("config/abfile_admin")
