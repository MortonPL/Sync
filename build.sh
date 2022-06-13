mkdir -p build
cd build
cmake ../
cmake --build .
cp src/Proto Proto
cd ..

cp -r rc/ build/.
