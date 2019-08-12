--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
-- ID for this server
server_id = "230a6dd3-907e-4193-87e5-a25789d68016"
location = "AT"
server_name = "AT1"

require("config/data_server")

data_dir = "data"
recordings_dir = "recordings"
record_games = false

base_port = 2749          -- 0xABE

ai_server = false

require("config/server")
require("config/login")
require("config/mechanics")
require("config/msg_server")
