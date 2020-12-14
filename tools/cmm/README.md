# cmm

Tool to create client mini maps from terrain and buildings. It supports up to
four layers.

This program is similar to `hmerge`. The difference is, layers have a base color
and the height values brightens or darkens this base color.

It uses alpha blending to combine the layers. The first layer should always be opaque,
layers 2 - 4 may have an alpha value, e.g:
~~~sh
$ cmm -W 257 -H 257 -X 1 -Y 0.2 -Z 1 -P 32 -L1 705e55:temple_of_athene_HeightField.png -L2 80808080:temple_of_athene_HeightField.png.obstacles -o Athena.xml.minimap.png
~~~
In this example L1 has `0x705e55` (some green brownish) as base color and L2 has
`0x08` (128 = 50%) as alpha value.

IMPORTANT: Sources for all layers should have the same size, width, height, number of values.
