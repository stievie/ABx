# Install/run

## Server

1. Build the server, see [BUILD.md](BUILD.md)
2. On Windows copy the *OpenSSL* and *PostgreSQL* client libraries to the `Bin` directory.
    * libeay32.dll (OpenSSL)
    * ssleay32.dll (OpenSSL)
    * libiconv-2.dll (PostgreSQL)
    * libintl-9.dll (PostgreSQL)
    * libpq.dll (PostgreSQL)
    * libwinpthread-1.dll (PostgreSQL)
3. Setup a PosgreSQL server
4. Create a `db_private.lua` file in `Bin/config` directory to enter the DB configuration, example:
~~~lua
-- Postgres
db_host = "database.host"
db_port = 5432
db_user = "user"
db_pass = "password"
~~~
5. Create a database with the name `forgottenwars`.
6. Create the DB structure with importing `sql/schema.*.sql` one by one in the proper order.
7. Download server assets `data` from [OneDrive)[https://1drv.ms/f/s!Ajy_fJI3BLBobOAOXZ47wtBgdBg] and put them into the `Bin/data` directory. Don't overwrite files that are in the git repository.
8. Run `run.bat` or `./run` in the root directory
9. You may want to create an account key to be able to create an account:
~~~sql
INSERT INTO public.account_keys VALUES ('(Random_Guid)', 0, 100, 'My Account Key', 2, 1, '');
~~~

## Client

1. Build it, see [BUILD.md](BUILD.md)
2. Download client assets `client_data.zip` from [OneDrive)[https://1drv.ms/f/s!Ajy_fJI3BLBobOAOXZ47wtBgdBg] and put them into the clients `abclient/bin` directory (don't replace existing files)
3. Run `fw.exe`
4. Create an account using the (Random_Guid) value of your previously created account key.
