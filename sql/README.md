# SQL

Directory for Database updates. The `dbtool` program takes this as default directory
for updates. Also the Data Server is using this directory to check if the Database
is up to date.

*Note* All statements in a file must be terminated with a semicolon (`;`), even
when a file contains just one statement.

## Naming convention

~~~plain
schema.<version>.sql
~~~

After importing the file, the value of the `schema` record in the `public.versions`
table will be `<version>`. So each file must have
~~~sql
UPDATE public.versions SET value = version WHERE name = 'schema';
~~~
at the end to match the file version.

## Fequently used

Make sure the merchant has some materials:

~~~sql
update concrete_items set count = 4999961 where item_uuid in(select uuid from game_items where type = 1000) and storage_place = 5
~~~
