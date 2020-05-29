--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "548350e7-5919-4bcf-a885-2d2888db05ca"
location = "AT"
server_name = "AB Admin"

admin_ip = ""         -- Listen on all
admin_host = ""       -- emtpy use same host as for login
admin_port = 443

-- Create self signing Key/Cert with something like this:
-- openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes -keyout server.key -out server.crt
server_key = "server.key"
server_cert = "server.crt"

-- Thread pool size
num_threads = 4
root_dir = "admin/root"
-- 60min
session_lifetime = 1000 * 60 * 60

require("config/data_server")
require("config/msg_server")
require("config/login")
