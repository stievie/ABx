--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------
-- ID for this server
server_id = "14935aef-7cd9-4495-b704-ec9578411558"
location = "AT"
server_name = "AT2"

require("config/data_server")

data_dir = "data"
recordings_dir = "recordings"
record_games = false

-- 2nd Game server. Must listen of different ports.
base_port = 2749 + 1

require("config/server")
require("config/login")
require("config/mechanics")
require("config/msg_server")

ai_server = false
