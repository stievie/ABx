--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
server_name = "Forgotten Wars"
-- ID for this server
server_id = "9b53f954-db11-413b-85b6-b7080b0f4063"

location = "AT"

-- Data server
data_host = "localhost"
data_port = 2770

data_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/data"

-- 2nd Game server. Must listen of different ports. One Game server listens usually
-- listens on 3 ports, so increase the base port by 3.
base_port = 2749 + 6

require("config/server")
require("config/login")
require("config/mechanics")
require("config/admin")
require("config/msg_server")
