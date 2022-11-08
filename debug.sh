mkdir -p debug
cd debug
cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake --build .
cp src/SyncGUI SyncGUI
cp src/SyncCLI SyncCLI
cd ..

cp -r rc/ debug/.
