mkdir -p build
cd build
cmake ../
cmake --build .
cp src/Sync Sync
cd ..

cp -r rc/ build/.
