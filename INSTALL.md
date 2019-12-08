# Install/run

## Server

1. Build the server, see [BUILD.md](BUILD.md)
2. Setup a PosgreSQL server
3. Create a `db_private.lua` file in `Bin/config` directory to enter the DB configuration, example:
~~~lua
-- Postgres
db_host = "database.host"
db_port = 5432
db_user = "user"
db_pass = "password"
~~~
4. Create a database with the name `forgottenwars`.
5. Create the DB structure with importing `sql/schema.*.sql`
6. Import `sql/data.*.sql`
7. Download server assets `data` from https://1drv.ms/f/s!Ajy_fJI3BLBobOAOXZ47wtBgdBg and put them into the `Bin/data` directory. Don't overwrite files that are in the git repository.
8. Run `run.bat` or `./run.sh` in the root directory
9. You may want to create an account key to be able to create an account:
~~~sql
INSERT INTO public.account_keys VALUES ('(Random_Guid)', 0, 100, 'My Account Key', 2, 1, '');
~~~

## Client

1. Build it, see [BUILD.md](BUILD.md)
2. Download client assets `client_data.zip` from https://1drv.ms/f/s!Ajy_fJI3BLBobOAOXZ47wtBgdBg and put them into `bin/AbData` (don't replace existing files)
3. Run `fw.exe`
4. Create an account using your previously created account key
