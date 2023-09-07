File is laid out in the following order:
## Header

|Bytes|Details|
|---|---|
|0 - 3|'S', 'A', \0, \0|
|4 - 7|Bone count|
|8 - 11|Keyframe count|

What follows is an array of bones of same size as specified in header. Each bone has an array of keyframes of same size as specified in header too.
## Bone

|Bytes|Details|
|---|---|
|0 - 63|4x4 matrix for keyframe 0|
|64 - 127|4x4 matrix for keyframe 1|
|128 - n|... Etc|
