# dbtool

Database administration tool. Since this program writes directly to the database
server, the data server must not run.

IMPORTANT: Whenever you use the `dbtool` the server must not be running. `dbtool`
directly writes to the database, and when the data server runs it may overwrite
what `dbtool` wrote.

## Usage

~~~plain
SYNOPSIS
    dbtool [-h] [<action>] [-r] [-v] [-dbdriver <dbdriver>] [-dbhost <dbhost>] [-dbport <dbport>] [-dbname <dbname>] [-dbuser <dbuser>] [-dbpass <dbpass>] [-d <schemadir>]

OPTIONS
    [-h, --help, -?]
        Show help
    [action <string>]
        What to do, possible value(s) see bellow
    [-r, --read-only]
        Do not write to Database
    [-v, --verbose]
        Write out stuff
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
        Directory with .sql files to import for updating

ACTIONS
    update       Update the database
    versions     Show database and table versions
    acckeys      Show account keys
    genacckey    Generate a new account key
    updateskills Update skills stats in DB

EXAMPLES
    dbtool update
    dbtool update -d "dir/with/sql/files"
~~~
