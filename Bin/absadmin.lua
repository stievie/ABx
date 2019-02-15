--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "548350e7-5919-4bcf-a885-2d2888db05ca"
location = "AT"
server_name = "AB Admin"

admin_ip = ""         -- Listen on all
admin_host = ""       -- emtpy use same host as for login
admin_port = 443

server_key = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/server.key"
server_cert = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/server.crt"

-- Thread pool size
num_threads = 4
root_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/admin/root"
-- 60min
session_lifetime = 1000 * 60 * 60

require("config/data_server")
require("config/msg_server")
require("config/login")
