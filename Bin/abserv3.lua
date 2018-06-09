--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
-- ID for this server
server_id = "9b53f954-db11-413b-85b6-b7080b0f4063"
location = "AT"
server_name = "AT3"

-- Data server
data_host = "localhost"
data_port = 2770

data_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/data"

-- 2nd Game server. Must listen of different ports. One Game server listens usually
-- listens on 3 ports, so increase the base port by 3.
base_port = 2749 + 2

require("config/server")
require("config/login")
require("config/mechanics")
require("config/admin")
require("config/msg_server")
