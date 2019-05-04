--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
-- ID for this server
server_id = "9b53f954-db11-413b-85b6-b7080b0f4063"
location = "AT"
server_name = "AT3"

require("config/data_server")

data_dir = "data"
recordings_dir = "recordings"
record_games = false

-- 2nd Game server. Must listen of different ports.
base_port = 2749 + 2

require("config/server")
require("config/login")
require("config/mechanics")
require("config/msg_server")
