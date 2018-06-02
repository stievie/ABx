--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
server_name = "Forgotten Wars"
-- ID for this server
server_id = "14935aef-7cd9-4495-b704-ec9578411558"

location = "AT"

-- Data server
data_host = "localhost"
data_port = 2770

data_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/data"

-- 2nd Game server. Must listen of different ports. One Game server listens usually
-- listens on 3 ports, so increase the base port by 3.
base_port = 2749 + 3

require("config/server")
require("config/login")
require("config/mechanics")
require("config/admin")
