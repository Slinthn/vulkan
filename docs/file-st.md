Terrain file

|Bytes|Details|
|---|---|
|0 - 3|'S', 'T', \0, \0|
|4 - 7|Terrain width|
|8 - 11|Terrain height|

Then the heights follow. They are all floats. There are `terrain width * terrain height` entries.
## Height

|Bytes|Details|
|---|---|
|0 - 3|Floating point height|
