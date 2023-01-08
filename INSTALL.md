## Instalowanie pakietów ogólnodostępnych
```
sudo apt-get install build-essential
sudo apt-get install libssh-dev
sudo apt-get install libgtk-3-dev
sudo apt-get install uuid-dev
sudo apt-get install cmake
```

## Własne budowanie wxWidgets
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

## Budowanie projektu
```
# Instrukcja użycia skryptu: ./action.sh -h
# Flaga -c wymusza (re)generację cache CMake
# Flaga -i powoduje, że po zbudowaniu, potrzebne pliki zostaną skopiowane do ~/.sync
./action.sh build -c -i
```

## Przygotowywanie programu do działania

Na maszynie lokalnej (tej, na której będzie uruchamiane GUI), musi znajdować się plik `~/.sync/res/sync.xrs`. Zawiera on zasoby, bez których program się nie uruchomi. Plik `sync.xrs` generowany jest w ramach skryptu `action.sh` i znajduje się wraz z plikami wykonywalnymi w katalogu `build` lub `debug`, zależnie od wybranej konfiguracji. Pliki wykonywalne mogą znajdować się w dowolnym miejscu, chociaż zalecany jest katalog `~/.sync/bin`.

Do poprawnego działania, na maszynie zdalnej (tej, z którą chcemy się synchronizować) podprogram `synccli` musi być dodany do ścieżki tak, aby był wykrywalny dla sesji nieinteraktywnej (np. tworząc dowiązanie `/usr/local/bin/synccli` wskazujące na podprogram). Na maszynie zdalnej powinien działać serwer SSH.

Uruchomienie programu `syncgui` uruchomi narzędzie z GUI.
