@echo off
cls

pushd ..\bin

py ../scripts/ply.py ../res/counter.ply counter.sm
py ../scripts/ply.py ../res/shelf.ply shelf.sm
py ../scripts/ply.py ../res/shop.ply shop.sm
py ../scripts/ply.py ../res/vending.ply vending.sm

py ../scripts/bmp.py ../res/cat.bmp cat.simg
py ../scripts/bmp.py ../res/dog.bmp dog.simg
py ../scripts/bmp.py ../res/wood.bmp wood.simg
py ../scripts/bmp.py ../res/vending.bmp vending.simg

py ../scripts/world.py ../scripts/world.json world.sw

popd
