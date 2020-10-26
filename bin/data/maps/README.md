# Scene

Directory containing the server scene files. Each map has its own subdirectory.

## Creating

All files required by the server can be created from Urho3D scene files with the `import` tool.

Example (run in `bin` directory):

~~~sh
$  import -scene "../abclient/bin/AbData/Scenes/Rhodes.xml" -o data/maps/rhodes
~~~

This generates all needed files and puts them into the `data/maps/rhodes`.

## Spawn Points

Spawn points (Position and Rotation) are Nodes with the name `SpawnPoint`.
