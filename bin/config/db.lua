--------------------------------------------------------------------------------
-- Database Settings -----------------------------------------------------------
--------------------------------------------------------------------------------

-- Database: mysql (default port 3306), pgsql (default port 5432), sqlite, odbc
db_driver = "pgsql"
-- Database name or DSN for ODBC
db_name = "forgottenwars"

-- Directory where SQL files are located
db_schema_dir = EXE_PATH .. "/../sql"

-- There should be the following formation in db_private.lua:
--  db_host = "database.host"
--  db_port = 5432
--  db_user = "username"
--  db_pass = "password"
require("config/db_private")
