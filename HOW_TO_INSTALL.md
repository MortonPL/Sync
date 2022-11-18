Instalowanie pakietów ogólnodostępnych
```
sudo apt-get install build-essential
sudo apt-get install libssh-dev
sudo apt-get install libgtk-3-dev
sudo apt-get install uuid-dev
```

Własne budowanie wxWidgets
```
git clone --recurse-submodules https://github.com/wxWidgets/wxWidgets.git
cd wxWidgets   
mkdir gtk-build             # the name is not really relevant
cd gtk-build
../configure
make -j3                    # use 3 cores. Set to the number of cores your have. 'make' uses 1 core
sudo make install
sudo ldconfig
```

Budowanie projektu
```
./build.sh
```
