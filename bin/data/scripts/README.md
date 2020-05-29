# Lua Scripts

Scripts are loaded, compiled and cached when they are used.

Instead of `require()` we must use the custom function `include()`.

## Built-in global functions

* `Tick()` return game tick
* `Random()` returns a random number between 0..1
* `ServerId()`
* `include(string)`

## Skills

Whenever Skill stats (energy cost, activation time etc) change, the database table
must be updated. Run in the `Bin` directory:
~~~sh
$ ./dbtool -a updateskills
~~~
