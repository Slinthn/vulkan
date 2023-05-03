clear

pushd ../bin

python3 ../scripts/ply.py ../res/counter.ply counter.sm
python3 ../scripts/ply.py ../res/shelf.ply shelf.sm
python3 ../scripts/ply.py ../res/shop.ply shop.sm

python3 ../scripts/world.py ../scripts/world.json ../bin/world.sw

python3 ../scripts/bmp.py ../res/cat.bmp ../bin/cat.simg
python3 ../scripts/bmp.py ../res/dog.bmp ../bin/dog.simg
python3 ../scripts/bmp.py ../res/wood.bmp ../bin/wood.simg

popd

