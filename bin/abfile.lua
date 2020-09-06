--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "0e9cf876-bb63-4926-a1a9-271bcf4a1c39"
location = "AT"
server_name = "generic"

file_ip = ""         -- Listen on all
file_host = ""       -- emtpy use same host as for login
file_port = 8081

-- Used to calculate the load Byte/sec (100Mbit)
max_throughput = (100 * 1024 * 1024)

-- Create self signing Key/Cert with something like this:
-- openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes -keyout server.key -out server.crt
server_key = "server.key"
server_cert = "server.crt"

-- Thread pool size
num_threads = 4
root_dir = "file_root"

-- If true client must give Account UUID and Auth token obtained from login server in the HTTP header:
-- Auth: UUID:AuthToken
require_auth = true

require("config/data_server")
require("config/msg_server")
require("config/login")
