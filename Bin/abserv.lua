--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
-- ID for this server
server_id = "230a6dd3-907e-4193-87e5-a25789d68016"
location = "AT"
server_name = "AT1"

-- Data server
data_host = "localhost"
data_port = 2770

data_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/data"
recordings_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/recordings"
record_games = true

base_port = 2749          -- 0xABE

require("config/server")
require("config/login")
require("config/mechanics")
require("config/admin")
require("config/msg_server")
