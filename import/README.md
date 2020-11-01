# import

Tool to import/create server assets.

## Import Urho3D Scene

This tool can create server scenes with all required files from Urho3D scenes.

Example:

~~~sh
$  import -scene "..\abclient\bin\AbData\Scenes\Rhodes.xml" -o test
~~~

Create all files in the test subdirectory. These files are:

* `index.xml` Meta data
* `Rhodes.xml` A copy of Urho3Ds scene file
* `Rhodes_Heightfield.png.hm` Terrain height field
* `Rhodes_Heightfield.png.navmesh` Navigation mesh created from the height field

There may be some other intermediate files which can be deleted:

* `Rhodes_Heightfield.png` Copy of Uhro3D height field
* `Rhodes_Heightfield.png.obstacles` Static objects in the scene
* `Rhodes_Heightfield.png.obj` The scene as Wavefront OBJ file

NOTE: Server scenes are located in the server `data/maps` directory, each scene
having its own subdirectory.
