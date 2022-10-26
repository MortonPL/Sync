mkdir -p build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build .
cp src/Sync Sync
cd ..

cp -r rc/ build/.
