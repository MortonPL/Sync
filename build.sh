mkdir -p build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build .
cp src/SyncGUI SyncGUI
cp src/SyncCLI SyncCLI
cd ..

cp -r rc/ build/.
