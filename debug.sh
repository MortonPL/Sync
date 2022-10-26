mkdir -p debug
cd debug
cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake --build .
cp src/Sync Sync
cd ..

cp -r rc/ debug/.
