@echo off
cls

pushd ..\bin

rem py ../scripts/ply.py ../res/counter.ply counter.sm
rem py ../scripts/ply.py ../res/shelf.ply shelf.sm
rem py ../scripts/ply.py ../res/shop.ply shop.sm
rem py ../scripts/ply.py ../res/vending.ply vending.sm
rem 
rem py ../scripts/bmp.py ../res/cat.bmp cat.simg
rem py ../scripts/bmp.py ../res/dog.bmp dog.simg
rem py ../scripts/bmp.py ../res/wood.bmp wood.simg
rem py ../scripts/bmp.py ../res/vending.bmp vending.simg
rem 
rem py ../scripts/world.py ../scripts/world.json world.sw

python ../scripts/terrain.py ../res/Heightmap.png terrain.st

popd
