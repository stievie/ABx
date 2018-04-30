--------------------------------------------------------------------------------
-- Database Settings -----------------------------------------------------------
--------------------------------------------------------------------------------

-- Database: mysql (default port 3306), pgsql (default port 5432), sqlite, odbc
db_driver = "mysql"
-- Database Host. Not used for ODBC
db_host = "ba.home.lan"
-- Database Port. Not used for ODBC
db_port = 3306
-- Database name or DSN for ODBC
db_name = "forgottenwars"

-- db_user = "username"
-- db_pass = "password"

require("config/db_private")
