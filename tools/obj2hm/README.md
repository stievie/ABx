# obj2hm

Program to construct a height map image from a 3D mesh in OBJ file format. This
works best with meshes created with the `import` program. It may not be able to
load OBJ files created with other programs.

It can output the height map a PNG picture and ABx' own `.hm` format. Additionally
the data can be saved in plain text and some JSON format to make it readable by
other programs.

## Usage

~~~sh
$ obj2hm inputfile.obj
~~~

This will create `inputfile.obj.png`.

## Example

Source height map:

![Source Map](source_map.png?raw=true)

With `genavmesh` generated 3D source mesh with added obstacles:

![Source Mesh](source_mesh.jpg?raw=true)

From source mesh generated height map, now includes the heights of the obstacles:

![Generated Map](generated_map.png?raw=true)
