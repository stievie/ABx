--------------------------------------------------------------------------------
-- abdata Settings -------------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "cd00b30c-fc7f-416d-be9e-cec9fb34fb79"
location = "AT"

-- If true it doesnt write to the DB
read_only = false

data_port = 2770
-- IP it listens on
data_ip = "127.0.0.1"

-- Max memory size, 1GB
max_size = 1024 * 1024 * 1024

require("config/db")
