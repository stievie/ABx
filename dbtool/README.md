# dbtool

Tool to update the Database.

## Usage

~~~plain
SYNOPSIS
    dbtool [-h] -a <action> [-dbdriver <dbdriver>] [-dbhost <dbhost>] [-dbport <dbport>] [-dbname <dbname>] [-dbuser <dbuser>] [-dbpass <dbpass>] [-d <schemadir>]

OPTIONS
    [-h, -help, -?]
        Show help
    -a, --action <string>
        What to do, possible value(s): update
    [-dbdriver, --database-driver <string>]
        Database driver, possible value(s): pgsql
    [-dbhost, --database-host <string>]
        Host name of database server
    [-dbport, --database-port <integer>]
        Port to connect to
    [-dbname, --database-name <string>]
        Name of database
    [-dbuser, --database-user <string>]
        User name for database
    [-dbpass, --database-password <string>]
        Password for database
    [-d, --schema-dir <string>]
        Directory with .sql files to import
~~~
