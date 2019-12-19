# SQL

Directory for Database updates. The `dbtool` program takes this as default directory
for updates.

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
