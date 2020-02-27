--------------------------------------------------------------------------------
-- Database Settings -----------------------------------------------------------
--------------------------------------------------------------------------------

-- Database: mysql (default port 3306), pgsql (default port 5432), sqlite, odbc
db_driver = "pgsql"
-- Database name or DSN for ODBC
db_name = "forgottenwars"

-- Database Host. Not used for ODBC
--db_host = "ba.home.lan"
-- Database Port. Not used for ODBC
--db_port = 3306

db_schema_dir = EXE_PATH .. "/../sql"

require("config/db_private")
