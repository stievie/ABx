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
* `Rhodes_Heightfield.png` A copy of the terrain height field
* `Rhodes_Heightfield.png.navmesh` Navigattion mesh created from the height field

NOTE: Server scenes are located in the server `data/maps` directory, each scene
having its own subdirectory.
