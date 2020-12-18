--------------------------------------------------------------------------------
-- Login Server Settings -------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "ef5ac3ec-45f9-40dc-8523-99c7937c69ba"
location = "AT"
server_name = "ablogin"

-- login server port
login_port = 2748         -- 0xABC
login_ip = "0.0.0.0"
--login_host = "stievie.mooo.com"

enable_ping_server = true

-- DOS prevention
max_packets_per_second = 60

require("config/data_server")
require("config/login")
require("config/msg_server")
