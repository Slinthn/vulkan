clear

pushd ../bin > /dev/null

../build/glslc -o shader-v.spv ../src/vulkan/glsl/shader.vert
../build/glslc -o shader-f.spv ../src/vulkan/glsl/shader.frag
../build/glslc -o shadow-v.spv ../src/vulkan/glsl/shadow.vert
../build/glslc -o shadow-f.spv ../src/vulkan/glsl/shadow.frag

gcc -g ../src/x11.c -lxcb -lm -l:libvulkan.so.1.3.245 -I /usr/include/ -DSLN_X11 -DSLN_VULKAN -DSLN_DEBUG -o x11

popd > /dev/null

