--------------------------------------------------------------------------------
-- ablb Settings ---------------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "961226da-0010-45c1-9141-b45872c46991"
location = "AT"
server_name = "ablb"

lb_host = "0.0.0.0"
lb_port = 2740
lb_type = 4      -- Load balancer for Login Server (= type 4)
-- If data_port is 0 a server list file must be given
server_list = ""

require("config/data_server")
