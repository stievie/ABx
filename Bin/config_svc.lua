--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
server_name = "Forgotten Wars"

location = "Austria"

-- Data server
data_host = "localhost"
data_port = 2770

log_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/logs/abserv"
data_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/data"

require("config/server")
require("config/login")
require("config/mechanics")
require("config/admin")
