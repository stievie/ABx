# Lua Scripts

Scripts are loaded, compiled and cached when they are used.

Instead of `require()` we must use the custom function `include()`.

## Built-in global functions

* `Tick()` return game tick
* `Random()` returns a random number between 0..1
* `ServerId()`
* `include(string)`
* `include_dir(string)` (should be avoided)
