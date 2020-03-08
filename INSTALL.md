# Install and run

## Server

1. Build the server, see [BUILD.md](BUILD.md)
2. On Windows copy the *OpenSSL* and *PostgreSQL* client libraries to the `Bin` directory.
    * `libeay32.dll` (OpenSSL)
    * `ssleay32.dll` (OpenSSL)
    * `libiconv-2.dll` (PostgreSQL)
    * `libintl-9.dll` (PostgreSQL)
    * `libpq.dll` (PostgreSQL)
    * `libwinpthread-1.dll` (PostgreSQL)
3. [Setup a PosgreSQL server](https://wiki.archlinux.org/index.php/PostgreSQL)
4. Create a `db_private.lua` file in `Bin/config` directory to enter the DB configuration, example:
~~~lua
-- Postgres
db_host = "database.host"
db_port = 5432
db_user = "user"
db_pass = "password"
~~~
5. Create a database with the name `forgottenwars`.
~~~sh
$ createdb forgottenwars
~~~
6. Create the DB structure with importing `sql/schema.*.sql` one by one in the proper order. Instead of importing the files manually, you can run `Bin/dbtool -a update`. If the DB is empty, `dbtool` emits *one* error, you can ignore this.
7. Download server assets `data` from [OneDrive](https://1drv.ms/f/s!Ajy_fJI3BLBobOAOXZ47wtBgdBg) and put them into the `Bin/data` directory. Don't overwrite files that are in the git repository.
8. Run the `Bin/keygen` tool to create the DH server keys.
9. Run `run.bat` or `./run` in the root directory, which runs all required services in the correct order.
10. You may want to create an account key to be able to create an account. You could use the `random_guid()` function to generate the GUID, e.g.:
~~~sql
INSERT INTO public.account_keys VALUES (random_guid(), 0, 100, 'My Account Key', 2, 1, '');
~~~
Or you can use the `dbtool`:
~~~sh
$ dbtool -a genacckey
~~~

### Known issuses

The Server should run on Windows and Linux.

## Client

1. Build it, see [BUILD.md](BUILD.md)
2. Download client assets `client_data.zip` from [OneDrive](https://1drv.ms/f/s!Ajy_fJI3BLBobOAOXZ47wtBgdBg) and put them into the clients `abclient/bin` directory (don't replace existing files). There should also be the Urho3D asset files, this directory should look like:
~~~plain
bin
  - AbData
  - Autoload
  - CoreData
  - Data
  - GameData
  - SoundData
~~~
3. On Windows copy the required libraries to the `abclient/bin` directory:
    * `d3dcompiler_47.dll`
    * `libeay32.dll` (OpenSSL)
    * `ssleay32.dll` (OpenSSL)
4. Copy `ablicnent/Setup/config.xml` to `ablicnent/bin/config.xml` and enter the Host and Port of the Login server:
~~~xml
<config>
  <parameter name="LoginHost" type="string" value="127.0.0.1" />
  <parameter name="LoginPort" type="int" value="2748" />
</config>
~~~
5. Run `fw.exe` in `abclient/bin`.
6. Create an account using the `random_guid()` value of your previously created account key. To find out what GUID was created, run:
~~~sql
SELECT uuid FROM public.account_keys;
~~~
Or use the `dbtool`:
~~~sh
$ dbtool -a acckeys
~~~

### Known issues

The client should work on Windows and Linux, however there are some minor issues
on Linux:

* Just closing the client does not log you out from the server, use Logout first and then close the client.
