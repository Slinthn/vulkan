@echo off
cls

pushd ..\bin

py ../scripts/ply.py ../res/counter.ply counter.sm
py ../scripts/ply.py ../res/shelf.ply shelf.sm
py ../scripts/ply.py ../res/shop.ply shop.sm

py ../scripts/world.py ../scripts/world.json ../bin/world.sw

py ../scripts/bmp.py ../res/cat.bmp ../bin/cat.simg
py ../scripts/bmp.py ../res/dog.bmp ../bin/dog.simg
py ../scripts/bmp.py ../res/wood.bmp ../bin/wood.simg

popd
