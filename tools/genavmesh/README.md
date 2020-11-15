# genavmesh

Generate Detour navmeshes from input geometry. This can be either an `.obj` file
or a `.png` heightmap. If it is a PNG file, the size should be a multiple of 
patch size + 1, otherwise it cuts off some pixels.

## Dependencies

* Recast/Detour
* stb_image
