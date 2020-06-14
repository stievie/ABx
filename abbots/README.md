# abbots

Client to login with many accounts. Will be used for stress tests.

This program spawns another instance for each account, because the client is not
made for being used with serveral accounts.

## Config

Place Login data in `./bin/config/bots_private.lua`:

~~~lua
account1 = {
    name = "botaccount1",
    pass = "password",
    char = "Name 1",
    script = "scripts/bots/nothing.lua"
}
account2 = {
    name = "botaccount2",
    pass = "password",
    char = "Name 2",
    script = "scripts/bots/nothing.lua"
}
account3 = {
    name = "botaccount3",
    pass = "password",
    char = "Name 3",
    script = "scripts/bots/nothing.lua"
}
account4 = {
    name = "botaccount4",
    pass = "password",
    char = "Name 4",
    script = "scripts/bots/nothing.lua"
}
~~~
